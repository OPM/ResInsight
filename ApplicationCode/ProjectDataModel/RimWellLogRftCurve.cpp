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

#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderEclipseRft.h"

#include "cafPdmObject.h"
#include "cafVecIjk.h"

#include "cvfAssert.h"

#include <qwt_plot.h>

#include <QString>

#include <numeric>
#include <vector>

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
RifEclipseRftAddress RimWellLogRftCurve::rftAddress() const
{
    return RifEclipseRftAddress(m_wellName, m_timeStep, m_wellLogChannelName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setDefaultAddress(QString wellName)
{
    RifReaderEclipseRft* reader = rftReader();
    if (!reader) return;

    bool wellNameHasRftData = false;
    std::set<QString> wellNames = reader->wellNames();
    for (const QString& wellNameWithRft : wellNames)
    {
        if (wellName == wellNameWithRft)
        {
            wellNameHasRftData = true;
            m_wellName = wellName;
            break;
        }
    }

    if (!wellNameHasRftData) return;

    m_wellLogChannelName = RifEclipseRftAddress::PRESSURE;

    std::vector<QDateTime> timeSteps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());
    if (timeSteps.size() > 0)
    {
        m_timeStep = timeSteps[0];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::updateWellChannelNameAndTimeStep()
{
    if (!m_timeStep().isValid() || m_wellLogChannelName() == RifEclipseRftAddress::NONE)
    {
        setDefaultAddress(m_wellName);
        return;
    }

    RifReaderEclipseRft* reader = rftReader();
    if (!reader) return;

    std::vector<QDateTime> timeSteps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());

    if (timeSteps.empty())
    {
        m_timeStep = QDateTime();
    }
    else if (std::find(timeSteps.begin(), timeSteps.end(), m_timeStep()) == timeSteps.end())
    {
        m_timeStep = timeSteps[0];
    }

    std::vector<RifEclipseRftAddress::RftWellLogChannelName> channelNames = reader->availableWellLogChannels(m_wellName);

    if (channelNames.empty())
    {
        m_wellLogChannelName = RifEclipseRftAddress::NONE;
    }
    else if (std::find(channelNames.begin(), channelNames.end(), m_wellLogChannelName()) == channelNames.end())
    {
        m_wellLogChannelName = RifEclipseRftAddress::PRESSURE;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::createCurveAutoName()
{
    QString name;
    if (m_eclipseResultCase)
    {
        name += m_eclipseResultCase->caseName();
    }
    if (wellName() != "")
    {
        name += ", ";
        name += wellName();
    }
    if (wellLogChannelName() != caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelName>::text(RifEclipseRftAddress::NONE))
    { 
        name += ", ";
        RifEclipseRftAddress::RftWellLogChannelName channelNameEnum = caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelName>::fromText(wellLogChannelName());
        name += caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelName>::uiText(channelNameEnum);
    }
    if ( !m_timeStep().isNull())
    {
        QString dateFormat = "dd MMM yyyy";
        name += ", ";
        name += m_timeStep().toString(dateFormat);
    }

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
        std::vector<double> tvDepthVector = tvDepthValues();
        std::vector<double> measuredDepthVector = measuredDepthValues();

        if (values.empty()) return;
        if (values.size() != tvDepthVector.size()) return;
        if (values.size() != measuredDepthVector.size()) return;
        
        m_curveData->setValuesWithTVD(values, measuredDepthVector, tvDepthVector, RiaEclipseUnitTools::depthUnit(m_eclipseResultCase->eclipseCaseData()->unitsType()), false);

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;
        if (wellLogPlot)
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        if (wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH)
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->measuredDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }
        else
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(), m_curveData->trueDepthPlotValues(displayUnit).data(), static_cast<int>(m_curveData->xPlotValues().size()));
        }

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
            QString dateFormat = "dd MMM yyyy";
            std::vector<QDateTime> timeStamps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());
            for (const QDateTime& dt : timeStamps)
            {
                options.push_back(caf::PdmOptionItemInfo(dt.toString(dateFormat), dt));
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
        m_timeStep = QDateTime();
        m_wellName = "";
        m_wellLogChannelName = RifEclipseRftAddress::NONE;

        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellName)
    {
        updateWellChannelNameAndTimeStep();
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
std::vector<double> RimWellLogRftCurve::tvDepthValues() const
{
    RifReaderEclipseRft* reader = rftReader();
    std::vector<double> tvDepthValues;

    if (!reader) return tvDepthValues;

    RifEclipseRftAddress depthAddress(m_wellName(), m_timeStep, RifEclipseRftAddress::DEPTH);

    reader->values(depthAddress, &tvDepthValues);
    return tvDepthValues;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::measuredDepthValues() const
{
    std::vector<double> measuredDepthForCells;
    
    RifReaderEclipseRft* reader = rftReader();
    if (!reader) return measuredDepthForCells;


    RimMainPlotCollection* mainPlotCollection;
    this->firstAncestorOrThisOfTypeAsserted(mainPlotCollection);

    RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();
    if (!wellLogCollection) return measuredDepthForCells;

    RigEclipseWellLogExtractor* eclExtractor = nullptr;

    RimProject* proj = RiaApplication::instance()->project();
    RimWellPath* wellPath = proj->wellPathFromSimulationWell(m_wellName());
    eclExtractor = wellLogCollection->findOrCreateExtractor(wellPath, m_eclipseResultCase);

    if (!eclExtractor)
    {
        std::vector<const RigWellPath*> wellPaths = proj->simulationWellBranches(m_wellName());
        if (wellPaths.size() == 0) return measuredDepthForCells;

        eclExtractor = wellLogCollection->findOrCreateSimWellExtractor(m_wellName(), QString("Find or create sim well extractor"), wellPaths[0], m_eclipseResultCase->eclipseCaseData());
    }

    if (!eclExtractor) return measuredDepthForCells;

    std::vector<double> measuredDepthForIntersections = eclExtractor->measuredDepth();
    
    std::vector< size_t> globCellIndices = eclExtractor->intersectedCellsGlobIdx();

    std::map<size_t, std::vector<double>> globCellIdToIntersectionDepthsMap;

    for (size_t iIdx = 0; iIdx < measuredDepthForIntersections.size(); ++iIdx)
    {
        globCellIdToIntersectionDepthsMap[globCellIndices[iIdx]].push_back(measuredDepthForIntersections[iIdx]);
    }

    const RigMainGrid* mainGrid = eclExtractor->caseData()->mainGrid();

    RifEclipseRftAddress depthAddress(m_wellName(), m_timeStep, RifEclipseRftAddress::DEPTH);
    std::vector<caf::VecIjk> indices;
    rftReader()->cellIndices(depthAddress, &indices);

    for (const caf::VecIjk& ijkIndex : indices)
    {
        size_t globalCellIndex = mainGrid->cellIndexFromIJK(ijkIndex.i(), ijkIndex.j(), ijkIndex.k());

        double sum = std::accumulate(globCellIdToIntersectionDepthsMap[globalCellIndex].begin(), globCellIdToIntersectionDepthsMap[globalCellIndex].end(), 0);

        measuredDepthForCells.push_back(sum / (double)globCellIdToIntersectionDepthsMap[globalCellIndex].size());
    }

    return measuredDepthForCells;
}
