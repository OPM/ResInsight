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

#pragma once

#include "RiaEclipseUnitTools.h"

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RicWellPathFractureReportItem
{
public:
    RicWellPathFractureReportItem(const QString& wellPathName, const QString& fractureName, const QString& fractureTemplateName, double measuredDepth);

    void setData(double trans, size_t connCount, double area);
    void setWidthAndConductivity(double width, double conductivity);
    void setHeightAndHalfLength(double height, double halfLength);
    void setAreaWeightedTransmissibility(double transmissibility);
    void setUnitSystem(RiaEclipseUnitTools::UnitSystem unitSystem);
    void setPressureDepletionParameters(bool performPressureDepletionScaling, QString timeStepString, QString wbhpString, double userWBHP, double actualWBHP, double minPressureDrop, double maxPressureDrop);

    QString wellPathNameForExport() const;
    QString fractureName() const;
    QString fractureTemplateName() const;

    RiaEclipseUnitTools::UnitSystem unitSystem() const;

    double transmissibility() const;
    size_t connectionCount() const;
    double fcd() const;
    double area() const;

    double kfwf() const;
    double kf() const;
    double wf() const;

    double xf() const;
    double h() const;
    double km() const;
    double kmxf() const;

    bool    performPressureDepletionScaling() const;
    QString pressureDepletionTimeStepString() const;
    QString pressureDepletionWBHPString() const;
    double  pressureDepletionUserWBHP() const;
    double  pressureDepletionActualWBHP() const;
    double  pressureDepletionMinPressureDrop() const;
    double  pressureDepletionMaxPressureDrop() const;

    bool operator < (const RicWellPathFractureReportItem& other) const;

private:
    RiaEclipseUnitTools::UnitSystem m_unitSystem;
    QString                         m_wellPathNameForExport;
    QString                         m_wellPathFracture;
    QString                         m_wellPathFractureTemplate;
    double                          m_mesuredDepth;

    double m_transmissibility;
    size_t m_connectionCount;
    double m_area;

    double m_kfwf;
    double m_kf;
    double m_wf;
    double m_xf;
    double m_h;
    double m_km;

    bool    m_performPressureDepletionScaling;
    QString m_pressureDepletionTimeStepString;
    QString m_pressureDepletionWBHPString;
    double  m_pressureDepletionUserWBHP;
    double  m_pressureDepletionActualWBHP;
    double  m_pressureDepletionMinPressureDrop;
    double  m_pressureDepletionMaxPressureDrop;
};
