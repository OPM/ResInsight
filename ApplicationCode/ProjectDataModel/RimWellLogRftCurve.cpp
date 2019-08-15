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
#include "RiaQDateTimeTools.h"
#include "RiaSimWellBranchTools.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderEclipseRft.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimTools.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPlotTools.h"

#include "RiuQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

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
void caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::setUp()
{
    addItem(RifEclipseRftAddress::NONE, "NONE", "None");
    addItem(RifEclipseRftAddress::TVD, "DEPTH", "Depth");
    addItem(RifEclipseRftAddress::PRESSURE, "PRESSURE", "Pressure");
    addItem(RifEclipseRftAddress::SWAT, "SWAT", "Water Saturation");
    addItem(RifEclipseRftAddress::SOIL, "SOIL", "Oil Saturation");
    addItem(RifEclipseRftAddress::SGAS, "SGAS", "Gas Saturation");
    addItem(RifEclipseRftAddress::WRAT, "WRAT", "Water Flow");
    addItem(RifEclipseRftAddress::ORAT, "ORAT", "Oil Flow");
    addItem(RifEclipseRftAddress::GRAT, "GRAT", "Gas flow");
    addItem(RifEclipseRftAddress::MD, "MD", "Measured Depth");
    addItem(RifEclipseRftAddress::PRESSURE_P10, "PRESSURE_P10", "Pressure P10");
    addItem(RifEclipseRftAddress::PRESSURE_P50, "PRESSURE_P50", "Pressure P50");
    addItem(RifEclipseRftAddress::PRESSURE_P90, "PRESSURE_P90", "Pressure P90");
    addItem(RifEclipseRftAddress::PRESSURE_MEAN, "PRESSURE_MEAN", "Pressure Mean");
    setDefault(RifEclipseRftAddress::NONE);
}
} // namespace caf

CAF_PDM_SOURCE_INIT(RimWellLogRftCurve, "WellLogRftCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::RimWellLogRftCurve()
{
    CAF_PDM_InitObject("Well Log RFT Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultCase, "CurveEclipseResultCase", "Eclipse Result Case", "", "", "");
    m_eclipseResultCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryCase, "CurveSummaryCase", "Summary Case", "", "", "");
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_ensemble, "CurveEnsemble", "Ensemble", "", "", "");
    m_ensemble.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_timeStep, "TimeStep", "Time Step", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "Well Name", "", "", "");
    CAF_PDM_InitField(&m_branchIndex, "BranchIndex", 0, "Branch Index", "", "", "");
    CAF_PDM_InitField(&m_branchDetection,
                      "BranchDetection",
                      true,
                      "Branch Detection",
                      "",
                      "Compute branches based on how simulation well cells are organized",
                      "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelName, "WellLogChannelName", "Well Property", "", "", "");

    m_isUsingPseudoLength = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve::~RimWellLogRftCurve() {}

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
void RimWellLogRftCurve::setSummaryCase(RimSummaryCase* summaryCase)
{
    m_summaryCase = summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimWellLogRftCurve::summaryCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setEnsemble(RimSummaryCaseCollection* ensemble)
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimWellLogRftCurve::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setRftAddress(RifEclipseRftAddress address)
{
    m_timeStep           = address.timeStep();
    m_wellName           = address.wellName();
    m_wellLogChannelName = address.wellLogChannel();
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
    RifReaderRftInterface* reader = rftReader();
    if (!reader) return;

    bool              wellNameHasRftData = false;
    std::set<QString> wellNames          = reader->wellNames();
    for (const QString& wellNameWithRft : wellNames)
    {
        if (wellName == wellNameWithRft)
        {
            wellNameHasRftData = true;
            m_wellName         = wellName;
            break;
        }
    }

    if (!wellNameHasRftData)
    {
        m_wellLogChannelName = RifEclipseRftAddress::NONE;
        m_timeStep           = QDateTime();
        return;
    }

    m_wellLogChannelName = RifEclipseRftAddress::PRESSURE;

    std::set<QDateTime> timeSteps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());
    if (!timeSteps.empty())
    {
        m_timeStep = *(timeSteps.begin());
    }
    else
    {
        m_timeStep = QDateTime();
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

    RifReaderRftInterface* reader = rftReader();
    if (!reader) return;

    std::set<RifEclipseRftAddress::RftWellLogChannelType> channelNames = reader->availableWellLogChannels(m_wellName);

    if (channelNames.empty())
    {
        m_wellLogChannelName = RifEclipseRftAddress::NONE;
    }
    else if (!channelNames.count(m_wellLogChannelName()))
    {
        m_wellLogChannelName = RifEclipseRftAddress::PRESSURE;
    }

    std::set<QDateTime> timeSteps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());

    if (timeSteps.empty())
    {
        m_timeStep = QDateTime();
    }
    else if (!timeSteps.count(m_timeStep()))
    {
        m_timeStep = *(timeSteps.begin());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::setSimWellBranchData(bool branchDetection, int branchIndex)
{
    m_branchDetection = branchDetection;
    m_branchIndex     = branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogRftCurve::createCurveAutoName()
{
    QStringList name;

    if (!wellName().isEmpty())
    {
        name.push_back(wellName());
    }

    name.push_back("RFT");

    if (m_eclipseResultCase)
    {
        name.push_back(m_eclipseResultCase->caseUserDescription());
    }
    else if (m_summaryCase)
    {
        name.push_back(m_summaryCase->caseName());
    }
    else if (m_ensemble)
    {
        name.push_back(m_ensemble->name());
    }
    if (wellLogChannelName() != caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::text(RifEclipseRftAddress::NONE))
    {
        RifEclipseRftAddress::RftWellLogChannelType channelNameEnum =
            caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::fromText(wellLogChannelName());
        name.push_back(caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(channelNameEnum));
    }
    if (!m_timeStep().isNull())
    {
        name.push_back(m_timeStep().toString(RiaQDateTimeTools::dateFormatString()));
    }

    return name.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    this->RimPlotCurve::updateCurvePresentation(updateParentPlot);

    m_isUsingPseudoLength = false;

    if (isCurveVisible())
    {
        m_curveData = new RigWellLogCurveData;

        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType(wellLogPlot);
        CVF_ASSERT(wellLogPlot);

        std::vector<double> measuredDepthVector = measuredDepthValues();
        std::vector<double> tvDepthVector       = tvDepthValues();
        std::vector<double> values              = xValues();

        if (values.empty() || values.size() != tvDepthVector.size())
        {
            this->detachQwtCurve();
            return;
        }

        RiaEclipseUnitTools::UnitSystem unitSystem = RiaEclipseUnitTools::UNITS_METRIC;
        if (m_eclipseResultCase)
        {
            unitSystem = m_eclipseResultCase->eclipseCaseData()->unitsType();
        }
        else if (m_summaryCase)
        {
            unitSystem = m_summaryCase->unitsSystem();
        }
		else if (m_ensemble)
		{
            unitSystem = m_ensemble->unitSystem();
		}
		else
        {
			CVF_ASSERT(false && "Need to have either an eclipse result case, a summary case or an ensemble");
		}

        if (tvDepthVector.size() != measuredDepthVector.size())
        {
            m_isUsingPseudoLength = true;
            measuredDepthVector   = tvDepthVector;
        }

        m_curveData->setValuesWithTVD(
            values, measuredDepthVector, tvDepthVector, RiaEclipseUnitTools::depthUnit(unitSystem), false);

        RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_METER;
        if (wellLogPlot)
        {
            displayUnit = wellLogPlot->depthUnit();
        }

        if (wellLogPlot->depthType() == RimWellLogPlot::MEASURED_DEPTH)
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(),
                                       m_curveData->measuredDepthPlotValues(displayUnit).data(),
                                       static_cast<int>(m_curveData->xPlotValues().size()));
        }
        else
        {
            m_qwtPlotCurve->setSamples(m_curveData->xPlotValues().data(),
                                       m_curveData->trueDepthPlotValues(displayUnit).data(),
                                       static_cast<int>(m_curveData->xPlotValues().size()));
            m_isUsingPseudoLength = false;
        }

        if (m_isUsingPseudoLength)
        {
            RimWellLogTrack* wellLogTrack;
            firstAncestorOrThisOfType(wellLogTrack);
            CVF_ASSERT(wellLogTrack);

            RiuWellLogTrack* viewer = wellLogTrack->viewer();
            if (viewer)
            {
                viewer->setDepthTitle("PL/" + wellLogPlot->depthPlotTitle());
            }
        }

        m_qwtPlotCurve->setLineSegmentStartStopIndices(m_curveData->polylineStartStopIndices());

        if (updateParentPlot)
        {
            updateZoomInParentPlot();
        }

        if (m_parentQwtPlot)
        {
            m_parentQwtPlot->replot();
        }
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

    RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromWellName(
        curveDataGroup, m_wellName, m_branchDetection, m_branchIndex);

    curveDataGroup->add(&m_wellLogChannelName);
    curveDataGroup->add(&m_timeStep);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogRftCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly)
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
        RifReaderRftInterface* reader = rftReader();
        if (reader)
        {
            std::set<QString> wellNames = reader->wellNames();
            for (const QString& name : wellNames)
            {
                options.push_back(caf::PdmOptionItemInfo(name, name, false, caf::QIconProvider(":/Well.png")));
            }
        }
    }
    else if (fieldNeedingOptions == &m_wellLogChannelName)
    {
        RifReaderRftInterface* reader = rftReader();
        if (reader)
        {
            for (const RifEclipseRftAddress::RftWellLogChannelType& channelName : reader->availableWellLogChannels(m_wellName))
            {
                options.push_back(caf::PdmOptionItemInfo(
                    caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(channelName), channelName));
            }
        }
        if (options.empty())
        {
            options.push_back(caf::PdmOptionItemInfo(
                caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(RifEclipseRftAddress::NONE),
                RifEclipseRftAddress::NONE));
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        RifReaderRftInterface* reader = rftReader();
        if (reader)
        {
            QString             dateFormat = "dd MMM yyyy";
            std::set<QDateTime> timeStamps = reader->availableTimeSteps(m_wellName, m_wellLogChannelName());
            for (const QDateTime& dt : timeStamps)
            {
                QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale(dt, dateFormat);

                options.push_back(caf::PdmOptionItemInfo(dateString, dt));
            }
        }

        options.push_back(caf::PdmOptionItemInfo("None", QDateTime()));
    }
    else if (fieldNeedingOptions == &m_branchIndex)
    {
        auto simulationWellBranches =
            RiaSimWellBranchTools::simulationWellBranches(RimWellPlotTools::simWellName(m_wellName), m_branchDetection);

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField(simulationWellBranches);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogRftCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue)
{
    m_idxInWellPathToIdxInRftFile.clear();

    RimWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);
    if (changedField == &m_eclipseResultCase)
    {
        m_timeStep           = QDateTime();
        m_wellName           = "";
        m_wellLogChannelName = RifEclipseRftAddress::NONE;

        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellName)
    {
        m_branchIndex = 0;

        updateWellChannelNameAndTimeStep();
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_branchDetection || changedField == &m_branchIndex)
    {
        QString simWellName = RimWellPlotTools::simWellName(m_wellName);

        m_branchIndex = RiaSimWellBranchTools::clampBranchIndex(simWellName, m_branchIndex, m_branchDetection);

        updateWellChannelNameAndTimeStep();
        this->loadDataAndUpdate(true);
    }
    else if (changedField == &m_wellLogChannelName)
    {
        if (m_wellLogChannelName == RifEclipseRftAddress::NONE)
        {
            m_timeStep = QDateTime();
        }
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
RifReaderRftInterface* RimWellLogRftCurve::rftReader() const
{
    if (m_eclipseResultCase())
    {
        return m_eclipseResultCase()->rftReader();
    }
    else if (m_summaryCase())
    {
        return m_summaryCase()->rftReader();
    }
    else if (m_ensemble())
    {
        return m_ensemble()->rftStatisticsReader();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogRftCurve::extractor()
{
    RifReaderRftInterface* reader = rftReader();
    if (!reader) return nullptr;

    RimMainPlotCollection* mainPlotCollection;
    this->firstAncestorOrThisOfTypeAsserted(mainPlotCollection);

    RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();
    if (!wellLogCollection) return nullptr;

    RigEclipseWellLogExtractor* eclExtractor = nullptr;

    RimProject*  proj     = RiaApplication::instance()->project();
    RimWellPath* wellPath = proj->wellPathFromSimWellName(m_wellName());
    eclExtractor          = wellLogCollection->findOrCreateExtractor(wellPath, m_eclipseResultCase);

    if (!eclExtractor)
    {
        QString                         simWellName = RimWellPlotTools::simWellName(m_wellName);
        std::vector<const RigWellPath*> wellPaths = RiaSimWellBranchTools::simulationWellBranches(simWellName, m_branchDetection);
        if (wellPaths.size() == 0) return nullptr;

        m_branchIndex = RiaSimWellBranchTools::clampBranchIndex(simWellName, m_branchIndex, m_branchDetection);

        auto wellPathBranch = wellPaths[m_branchIndex];

        eclExtractor = wellLogCollection->findOrCreateSimWellExtractor(
            simWellName, QString("Find or create sim well extractor"), wellPathBranch, m_eclipseResultCase->eclipseCaseData());
        m_isUsingPseudoLength = true;
    }

    return eclExtractor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogRftCurve::createWellPathIdxToRftFileIdxMapping()
{
    if (!m_idxInWellPathToIdxInRftFile.empty())
    {
        return true;
    }

    RigEclipseWellLogExtractor* eclExtractor = extractor();

    if (!eclExtractor) return false;

    std::vector<WellPathCellIntersectionInfo> intersections = eclExtractor->cellIntersectionInfosAlongWellPath();
    if (intersections.empty()) return false;

    std::map<size_t, size_t> globCellIndicesToIndexInWell;

    for (size_t idx = 0; idx < intersections.size(); idx++)
    {
        globCellIndicesToIndexInWell[intersections[idx].globCellIndex] = idx;
    }

    RifEclipseRftAddress     depthAddress(m_wellName(), m_timeStep, RifEclipseRftAddress::TVD);
    std::vector<caf::VecIjk> rftIndices;
    RifReaderEclipseRft*     eclipseRftReader = dynamic_cast<RifReaderEclipseRft*>(rftReader());
    if (!eclipseRftReader) return false;

    eclipseRftReader->cellIndices(depthAddress, &rftIndices);

    const RigMainGrid* mainGrid = eclExtractor->caseData()->mainGrid();

    for (size_t idx = 0; idx < rftIndices.size(); idx++)
    {
        caf::VecIjk ijkIndex        = rftIndices[idx];
        size_t      globalCellIndex = mainGrid->cellIndexFromIJK(ijkIndex.i(), ijkIndex.j(), ijkIndex.k());

        if (globCellIndicesToIndexInWell.find(globalCellIndex) != globCellIndicesToIndexInWell.end())
        {
            m_idxInWellPathToIdxInRftFile[globCellIndicesToIndexInWell[globalCellIndex]] = idx;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellLogRftCurve::rftFileIndex(size_t wellPathIndex)
{
    if (m_idxInWellPathToIdxInRftFile.empty())
    {
        createWellPathIdxToRftFileIdxMapping();
    }

    if (m_idxInWellPathToIdxInRftFile.find(wellPathIndex) == m_idxInWellPathToIdxInRftFile.end())
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    return m_idxInWellPathToIdxInRftFile[wellPathIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimWellLogRftCurve::sortedIndicesInRftFile()
{
    if (m_idxInWellPathToIdxInRftFile.empty())
    {
        createWellPathIdxToRftFileIdxMapping();
    }

    std::vector<size_t> indices;
    for (auto it = m_idxInWellPathToIdxInRftFile.begin(); it != m_idxInWellPathToIdxInRftFile.end(); it++)
    {
        indices.push_back(it->second);
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::xValues()
{
    RifReaderRftInterface* reader = rftReader();
    std::vector<double>    values;

    if (!reader) return values;

    RifEclipseRftAddress address(m_wellName(), m_timeStep, m_wellLogChannelName());

    reader->values(address, &values);

    bool wellPathExists = createWellPathIdxToRftFileIdxMapping();

    if (wellPathExists)
    {
        std::vector<double> valuesSorted;

        for (size_t idx : sortedIndicesInRftFile())
        {
            if (idx < values.size())
            {
                valuesSorted.push_back((values.at(idx)));
            }
        }

        return valuesSorted;
    }
    else
    {
        return values;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::tvDepthValues()
{
    RifReaderRftInterface* reader = rftReader();
    std::vector<double>    values;

    if (!reader) return values;

    RifEclipseRftAddress depthAddress(m_wellName(), m_timeStep, RifEclipseRftAddress::TVD);
    reader->values(depthAddress, &values);

    bool wellPathExists = createWellPathIdxToRftFileIdxMapping();

    if (wellPathExists)
    {
        std::vector<double> valuesSorted;

        for (size_t idx : sortedIndicesInRftFile())
        {
            if (idx < values.size())
            {
                valuesSorted.push_back((values.at(idx)));
            }
        }

        return valuesSorted;
    }
    else
    {
        return values;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellLogRftCurve::measuredDepthValues()
{
    std::vector<double> measuredDepthForCells;

    RigEclipseWellLogExtractor* eclExtractor = extractor();

    if (!eclExtractor) return measuredDepthForCells;

    std::vector<double> measuredDepthForIntersections = eclExtractor->cellIntersectionMDs();

    if (measuredDepthForIntersections.empty())
    {
        return measuredDepthForCells;
    }

    std::vector<size_t> globCellIndices = eclExtractor->intersectedCellsGlobIdx();

    for (size_t i = 0; i < globCellIndices.size() - 1; i = i + 2)
    {
        double sum = measuredDepthForIntersections[i] + measuredDepthForIntersections[i + 1];

        measuredDepthForCells.push_back(sum / 2.0);
    }

    std::vector<double> measuredDepthForCellsWhichHasRftData;

    for (size_t i = 0; i < measuredDepthForCells.size(); i++)
    {
        if (rftFileIndex(i) != cvf::UNDEFINED_SIZE_T)
        {
            measuredDepthForCellsWhichHasRftData.push_back(measuredDepthForCells[i]);
        }
    }

    return measuredDepthForCellsWhichHasRftData;
}
