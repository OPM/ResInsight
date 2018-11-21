/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicWellPathFractureReportItem.h"

#include "RigTransmissibilityEquations.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathFractureReportItem::RicWellPathFractureReportItem(const QString& wellPathName,
                                                             const QString& fractureName,
                                                             const QString& fractureTemplateName,
                                                             double         measuredDepth)
    : m_wellPathNameForExport(wellPathName)
    , m_wellPathFracture(fractureName)
    , m_wellPathFractureTemplate(fractureTemplateName)
    , m_mesuredDepth(measuredDepth)
    , m_transmissibility(0.0)
    , m_connectionCount(0)
    , m_area(0.0)
    , m_kfwf(0.0)
    , m_kf(0.0)
    , m_wf(0.0)
    , m_xf(0.0)
    , m_h(0.0)
    , m_km(0.0)
    , m_performPressureDepletionScaling(false)
    , m_pressureDepletionUserWBHP(0.0)
    , m_pressureDepletionActualWBHP(0.0)
    , m_pressureDepletionMinPressureDrop(-1.0)
    , m_pressureDepletionMaxPressureDrop(-1.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setData(double trans, size_t connCount, double area)
{
    m_transmissibility = trans;
    m_connectionCount  = connCount;
    m_area             = area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setWidthAndConductivity(double width, double conductivity)
{
    m_wf = width;
 
    double permeability = RigTransmissibilityEquations::permeability(conductivity, width);
    m_kf = permeability;

    m_kfwf = conductivity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setHeightAndHalfLength(double height, double halfLength)
{
    m_h  = height;
    m_xf = halfLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setAreaWeightedTransmissibility(double transmissibility)
{
    m_km = transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureReportItem::wellPathNameForExport() const
{
    return m_wellPathNameForExport;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureReportItem::fractureName() const
{
    return m_wellPathFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureReportItem::fractureTemplateName() const
{
    return m_wellPathFractureTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    m_unitSystem = unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setPressureDepletionParameters(bool performPDDScaling, QString timeStepString, QString wbhpString, double userWBHP, double actualWBHP, double minPressureDrop, double maxPressureDrop)
{
    m_performPressureDepletionScaling  = performPDDScaling;
    m_pressureDepletionTimeStepString  = timeStepString;
    m_pressureDepletionWBHPString      = wbhpString;
    m_pressureDepletionUserWBHP        = userWBHP;
    m_pressureDepletionActualWBHP      = actualWBHP;
    m_pressureDepletionMinPressureDrop = minPressureDrop;
    m_pressureDepletionMaxPressureDrop = maxPressureDrop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RicWellPathFractureReportItem::unitSystem() const
{
    return m_unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::transmissibility() const
{
    return m_transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellPathFractureReportItem::connectionCount() const
{
    return m_connectionCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::fcd() const
{
    double myFcd = 0.0;

    double threshold = 1.0e-7;
    if (fabs(kmxf()) > threshold)
    {
        myFcd = kfwf() / kmxf();
    }

    return myFcd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::area() const
{
    return m_area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::kfwf() const
{
    return m_kfwf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::kf() const
{
    return m_kf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::wf() const
{
    return m_wf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::xf() const
{
    return m_xf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::h() const
{
    return m_h;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::km() const
{
    return m_km;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::kmxf() const
{
    return m_km * m_xf;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathFractureReportItem::performPressureDepletionScaling() const
{
    return m_performPressureDepletionScaling;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureReportItem::pressureDepletionTimeStepString() const
{
    return m_pressureDepletionTimeStepString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellPathFractureReportItem::pressureDepletionWBHPString() const
{
    return m_pressureDepletionWBHPString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::pressureDepletionUserWBHP() const
{
    return m_pressureDepletionUserWBHP;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::pressureDepletionActualWBHP() const
{
    return m_pressureDepletionActualWBHP;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::pressureDepletionMinPressureDrop() const
{
    return m_pressureDepletionMinPressureDrop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathFractureReportItem::pressureDepletionMaxPressureDrop() const
{
    return m_pressureDepletionMaxPressureDrop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathFractureReportItem::operator<(const RicWellPathFractureReportItem& other) const
{
    if (this->wellPathNameForExport() != other.wellPathNameForExport())
    {
        return this->wellPathNameForExport() < other.wellPathNameForExport();
    }

    return this->m_mesuredDepth < other.m_mesuredDepth;
}
