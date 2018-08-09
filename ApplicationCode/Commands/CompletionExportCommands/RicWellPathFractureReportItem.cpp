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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathFractureReportItem::RicWellPathFractureReportItem(const QString& wellPathName,
                                                             const QString& fractureName,
                                                             const QString& fractureTemplateName)
    : m_wellPath(wellPathName)
    , m_wellPathFracture(fractureName)
    , m_wellPathFractureTemplate(fractureTemplateName)
    , m_transmissibility(0.0)
    , m_connectionCount(0)
    , m_fcd(0.0)
    , m_area(0.0)
    , m_kfwf(0.0)
    , m_kf(0.0)
    , m_wf(0.0)
    , m_xf(0.0)
    , m_h(0.0)
    , m_km(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setData(double trans, size_t connCount, double fcd, double area)
{
    m_transmissibility = trans;
    m_connectionCount  = connCount;
    m_fcd              = fcd;
    m_area             = area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::setWidthAndConductivity(double width, double conductivity)
{
    m_wf = width;
    m_kf = conductivity;

    m_kfwf = m_kf * m_wf;
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
QString RicWellPathFractureReportItem::wellPathName() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureReportItem::getNames(QString& wellPathName, QString& fractureName, QString& fractureTemplateName) const
{
    wellPathName         = m_wellPath;
    fractureName         = m_wellPathFracture;
    fractureTemplateName = m_wellPathFractureTemplate;
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
    return m_fcd;
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
