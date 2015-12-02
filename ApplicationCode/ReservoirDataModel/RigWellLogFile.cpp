/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigWellLogFile.h"

#include "RimWellLogCurve.h"

#include "well.hpp"
#include "laswell.hpp"

#include <QString>
#include <QFileInfo>

#include <exception>
#include <cmath> // Needed for HUGE_VAL on Linux

#define RIG_WELL_FOOTPERMETER 3.2808399


//--------------------------------------------------------------------------------------------------
/// Find the largest possible "ususal" value to use for absent data (-999.25, -9999.25, etc.)
//--------------------------------------------------------------------------------------------------
static double sg_createAbsentValue(double lowestDataValue)
{
    double absentValue = -999.0;

    while (absentValue > lowestDataValue)
    {
        absentValue *= 10;
        absentValue -= 9;
    }

    return absentValue - 0.25;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogFile::RigWellLogFile()
    : cvf::Object()
{
    m_wellLogFile = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogFile::~RigWellLogFile()
{
    close();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellLogFile::open(const QString& fileName, QString* errorMessage)
{
    close();

    NRLib::Well* well = NULL;

    try
    {
        int wellFormat = NRLib::Well::LAS;
        well = NRLib::Well::ReadWell(fileName.toStdString(), wellFormat);
        if (!well)
        {
            return false;
        }
    }
    catch (std::exception& e)
    {
        if (well)
        {
            delete well;
        }

        if (e.what())
        {
            CVF_ASSERT(errorMessage);
            *errorMessage = e.what();
        }

        return false;
    }

    QStringList wellLogNames;

    const std::map<std::string, std::vector<double> >& contLogs = well->GetContLog();
    std::map<std::string, std::vector<double> >::const_iterator itCL;
    for (itCL = contLogs.begin(); itCL != contLogs.end(); ++itCL)
    {
        QString logName = QString::fromStdString(itCL->first);
        wellLogNames.append(logName);

        if (logName.toUpper() == "DEPT" || logName.toUpper() == "DEPTH")
        {
            m_depthLogName = logName;
        }
    }

    m_wellLogChannelNames = wellLogNames;
    m_wellLogFile = well;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogFile::close()
{
    if (m_wellLogFile)
    {
        delete m_wellLogFile;
        m_wellLogFile = NULL;
    }

    m_wellLogChannelNames.clear();
    m_depthLogName.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::wellName() const
{
    CVF_ASSERT(m_wellLogFile);
    return QString::fromStdString(m_wellLogFile->GetWellName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RigWellLogFile::wellLogChannelNames() const
{
    return m_wellLogChannelNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogFile::depthValues() const
{
    return values(m_depthLogName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogFile::values(const QString& name) const
{
    CVF_ASSERT(m_wellLogFile);

    if (m_wellLogFile->HasContLog(name.toStdString()))
    {
        if (name == m_depthLogName && (depthUnitString().toUpper() == "F" || depthUnitString().toUpper() == "FT"))
        {
            std::vector<double> footValues = m_wellLogFile->GetContLog(name.toStdString());
            
            std::vector<double> meterValues;
            meterValues.reserve(footValues.size());

            for (size_t vIdx = 0; vIdx < footValues.size(); vIdx++)
            {
                meterValues.push_back(footValues[vIdx]/RIG_WELL_FOOTPERMETER);
            }

            return meterValues;
        }

        std::vector<double> values = m_wellLogFile->GetContLog(name.toStdString());
        
        for (size_t vIdx = 0; vIdx < values.size(); vIdx++)
        {
            if (m_wellLogFile->IsMissing(values[vIdx]))
            {
                // Convert missing ("NULL") values to HUGE_VAL
                values[vIdx] = HUGE_VAL;
            }
        }

        return values;
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::depthUnitString() const
{
    QString unit;

    NRLib::LasWell* lasWell = dynamic_cast<NRLib::LasWell*>(m_wellLogFile);
    if (lasWell)
    {
        unit = QString::fromStdString(lasWell->depthUnit());
    }

    return unit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::wellLogChannelUnitString(const QString& wellLogChannelName) const
{
    QString unit;

    NRLib::LasWell* lasWell = dynamic_cast<NRLib::LasWell*>(m_wellLogFile);
    if (lasWell)
    {
        unit = QString::fromStdString(lasWell->unitName(wellLogChannelName.toStdString()));
    }

    // Special handling of depth unit - we convert depth to meter 
    if (unit == depthUnitString())
    {
        return "m";
    }

    return unit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellLogFile::exportToLasFile(const RimWellLogCurve* curve, const QString& fileName)
{
    CVF_ASSERT(curve);

    const RigWellLogCurveData* curveData = curve->curveData();
    if (!curveData)
    {
        return false;
    }

    double minX, maxX;
    curve->valueRange(&minX, &maxX);
    double absentValue = sg_createAbsentValue(minX);

    std::vector<double> wellLogValues = curveData->xValues();
    for (size_t vIdx = 0; vIdx < wellLogValues.size(); vIdx++)
    {
        double value = wellLogValues[vIdx];
        if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
        {
            wellLogValues[vIdx] = absentValue;
        }
    }

    QString wellLogChannelName = curve->wellLogChannelName().trimmed();
    wellLogChannelName.replace(".", "_");

    QString wellLogDate = curve->wellDate().trimmed();
    wellLogDate.replace(".", "_");
    wellLogDate.replace(" ", "_");

    NRLib::LasWell lasFile;
    lasFile.addWellInfo("WELL", curve->wellName().trimmed().toStdString());
    lasFile.addWellInfo("DATE", wellLogDate.toStdString());
    lasFile.AddLog("DEPTH", "M", "Depth in meters", curveData->measuredDepths());
    lasFile.AddLog(wellLogChannelName.trimmed().toStdString(), "NO_UNIT", "", wellLogValues);
    lasFile.SetMissing(absentValue);

    double minDepth = 0.0;
    double maxDepth = 0.0;
    curveData->calculateMDRange(&minDepth, &maxDepth);

    lasFile.setStartDepth(minDepth);
    lasFile.setStopDepth(maxDepth);
    lasFile.setDepthUnit("M");

    lasFile.setVersionInfo("2.0");

    std::vector<std::string> commentHeader;
    lasFile.WriteToFile(fileName.toStdString(), commentHeader);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDefines::DepthUnitType RigWellLogFile::depthUnit() const
{
    RimDefines::DepthUnitType unitType = RimDefines::UNIT_METER;

    if (depthUnitString().toUpper() == "F" || depthUnitString().toUpper() == "FT")
    {
        unitType = RimDefines::UNIT_FEET;
    }

    return unitType;
}
