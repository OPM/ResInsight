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

#include "Rim3dWellLogRftCurve.h"

#include "RimWellPath.h"
#include "RimWellLogCurve.h"
#include "RigWellLogCurveData.h"
#include "RimEclipseResultCase.h"
#include "RifReaderEclipseRft.h"
#include "RimTools.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogRftCurve, "Rim3dWellLogRftCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogRftCurve::Rim3dWellLogRftCurve()
{
    CAF_PDM_InitObject("3d Well Log RFT Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultCase, "eclipseResultCase", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeStep, "timeStep", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelName, "wellLogChannelName", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_2dWellLogRftCurve, "my2dWellLogRftCurve", "", "", "", "");

    m_2dWellLogRftCurve = new RimWellLogRftCurve();
    m_2dWellLogRftCurve.xmlCapability()->disableIO();

    m_name = "3D Well Log RFT Curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogRftCurve::~Rim3dWellLogRftCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogRftCurve::curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const
{
    if (m_2dWellLogRftCurve->wellName() == QString())
    {
        m_2dWellLogRftCurve->setDefaultAddress(wellName());
    }

    const RigWellLogCurveData* curveData = m_2dWellLogRftCurve->curveData();

    //These values are for a simulation well
    *values = curveData->xValues();
    *measuredDepthValues = curveData->measuredDepths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogRftCurve::resultPropertyString() const
{
    return caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(m_wellLogChannelName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogRftCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue)
{
    if (changedField == &m_wellLogChannelName)
    {
        this->resetMinMaxValuesAndUpdateUI();
    }
    Rim3dWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogRftCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_eclipseResultCase)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_wellLogChannelName)
    {
        if (m_eclipseResultCase)
        {
            RifReaderEclipseRft* reader = m_eclipseResultCase()->rftReader();
            if (reader)
            {
                for (const RifEclipseRftAddress::RftWellLogChannelType& channelName : reader->availableWellLogChannels(wellName()))
                {
                    options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(channelName), channelName));
                }
            }
            if (options.empty())
            {
                options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(RifEclipseRftAddress::NONE), RifEclipseRftAddress::NONE));
            }
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        if (m_eclipseResultCase)
        {
            RifReaderEclipseRft* reader = m_eclipseResultCase()->rftReader();
            if (reader)
            {
                QString dateFormat = "dd MMM yyyy";
                std::vector<QDateTime> timeStamps = reader->availableTimeSteps(wellName(), m_wellLogChannelName());
                for (const QDateTime& dt : timeStamps)
                {
                    options.push_back(caf::PdmOptionItemInfo(dt.toString(dateFormat), dt));
                }
            }

            options.push_back(caf::PdmOptionItemInfo("None", QDateTime()));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogRftCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_eclipseResultCase);
    curveDataGroup->add(&m_wellLogChannelName);
    curveDataGroup->add(&m_timeStep);

    Rim3dWellLogCurve::configurationUiOrdering(uiOrdering);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogRftCurve::wellName() const
{
    RimWellPath* wellPath = nullptr;
    firstAncestorOrThisOfType(wellPath);

    return wellPath->name();
}
