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

#include "RiaEclipseUnitTools.h"

#include "RimEclipseResultCase.h"
#include "RimTools.h"
#include "RimWellLogPlot.h"

#include "RigEclipseCaseData.h"
#include "RigWellLogCurveData.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "RifReaderEclipseRft.h"
#include "RifEclipseRftAddress.h"

#include "cafPdmObject.h"
#include "cvfAssert.h"

#include <qwt_plot.h>

#include <QString>

namespace caf
{
    template<>
    void caf::AppEnum< RifEclipseRftAddress::RftWellLogChannelName >::setUp()
    {
        addItem(RifEclipseRftAddress::NONE, "NONE", "None");
        addItem(RifEclipseRftAddress::DEPTH, "DEPTH", "Depth");
        addItem(RifEclipseRftAddress::PRESSURE, "PRESSURE", "Pressure");
        addItem(RifEclipseRftAddress::SWAT, "SWAT", "Water Saturation");
        addItem(RifEclipseRftAddress::SOIL, "SOIL", "Oil Saturation");
        addItem(RifEclipseRftAddress::SGAS, "SGAS", "Gas Saturation");
        addItem(RifEclipseRftAddress::WRAT, "WRAT", "Water Flow");
        addItem(RifEclipseRftAddress::ORAT, "ORAT", "Oil Flow");
        addItem(RifEclipseRftAddress::GRAT, "GRAT", "Gas flow");
        setDefault(RifEclipseRftAddress::NONE);
    }
}

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
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::wellLogChannelName() const
{
    return m_wellLogChannelName().text();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setEclipseResultCase(RimEclipseResultCase* eclipseResultCase)
{
    m_eclipseResultCase = eclipseResultCase;
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
void RimWellLogRftCurve::setRftAddress(RifEclipseRftAddress address)
{
    m_timeStep = address.timeStep();
    m_wellName = address.wellName();
    m_wellLogChannelName = address.wellLogChannelName();
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
    RimWellLogCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        m_curveData = new RigWellLogCurveData;

        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);

        std::vector<double> values = xValues();
        std::vector<double> depthVector = depthValues();

        if (values.empty() || depthVector.empty()) return;

        if (values.size() == depthVector.size())
        {
            m_curveData->setValuesAndMD(values, depthVector, RiaEclipseUnitTools::depthUnit(m_eclipseResultCase->eclipseCaseData()->unitsType()), false);
        }

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;
        if (wellLogPlot)
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->measuredDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        m_qwtPlotCurve->setLineSegmentStartStopIndices(m_curveData->polylineStartStopIndices());

        updateZoomInParentPlot();
        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_eclipseResultCase);
    curveDataGroup->add(&m_wellName);
    curveDataGroup->add(&m_wellLogChannelName);
    curveDataGroup->add(&m_timeStep);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);
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
    else if (fieldNeedingOptions == &m_wellName)
    {
        options.push_back(caf::PdmOptionItemInfo("None", ""));
        RifReaderEclipseRft* reader = rftReader();
        if (reader)
        {
            std::set<QString> wellNames = reader->wellNames();
            for (const QString& name : wellNames)
            {
                options.push_back(caf::PdmOptionItemInfo(name, name, false, QIcon(":/Well.png")));
            }
        }
    }
    else if (fieldNeedingOptions == &m_wellLogChannelName)
    {
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelName>::uiText(RifEclipseRftAddress::NONE), RifEclipseRftAddress::NONE));
        RifReaderEclipseRft* reader = rftReader();
        if (reader)
        {
            for (const RifEclipseRftAddress::RftWellLogChannelName& channelName : reader->availableWellLogChannels(m_wellName))
            {
                options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelName>::uiText(channelName), channelName));
            }
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        RifReaderEclipseRft* reader = rftReader();
        if (reader)
        {
            std::vector<QDateTime> timeStamps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());
            for (const QDateTime& dt : timeStamps)
            {
                options.push_back(caf::PdmOptionItemInfo(dt.toString(), dt));
            }
        }

        options.push_back(caf::PdmOptionItemInfo("None", QDateTime()));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);
    if (changedField == &m_eclipseResultCase)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellName)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellLogChannelName)
    {
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_timeStep)
    {
        this->loadDataAndUpdate(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft* RimWellLogRftCurve::rftReader() const 
{
    if (!m_eclipseResultCase()) return nullptr;

    return m_eclipseResultCase()->rftReader();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::xValues() const
{
    RifReaderEclipseRft* reader = rftReader();
    std::vector<double> values;

    if (!reader) return values;

    RifEclipseRftAddress address(m_wellName(), m_timeStep, m_wellLogChannelName());

    reader->values(address, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::depthValues() const
{
    RifReaderEclipseRft* reader = rftReader();
    std::vector<double> values;

    if (!reader) return values;

    RifEclipseRftAddress address(m_wellName(), m_timeStep, RifEclipseRftAddress::DEPTH);

    reader->values(address, &values);

    return values;
}