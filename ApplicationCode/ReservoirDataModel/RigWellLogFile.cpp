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

#include "RigWellLogCurveData.h"

#include "RiaStringEncodingTools.h"

#include "RimWellLogCurve.h"

#include "well.hpp"
#include "laswell.hpp"

#include <QString>
#include <QFileInfo>

#include <exception>
#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogFile::RigWellLogFile()
    : cvf::Object()
{
    m_wellLogFile = nullptr;
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

    NRLib::Well* well = nullptr;

    try
    {
        int wellFormat = NRLib::Well::LAS;

        well = NRLib::Well::ReadWell(RiaStringEncodingTools::toNativeEncoded(fileName).data(), wellFormat);
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

        // 2018-11-09 Added MD https://github.com/OPM/ResInsight/issues/3641
        if (logName.toUpper() == "DEPT" || logName.toUpper() == "DEPTH" || logName.toUpper() == "MD")
        {
            m_depthLogName = logName;
        }
        else if (logName.toUpper() == "TVDMSL")
        {
            m_tvdMslLogName = logName;
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
        m_wellLogFile = nullptr;
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
    return RiaStringEncodingTools::fromNativeEncoded(m_wellLogFile->GetWellName().data());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::date() const
{
    CVF_ASSERT(m_wellLogFile);
    return QString::fromStdString(m_wellLogFile->GetDate());
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
std::vector<double> RigWellLogFile::tvdMslValues() const
{
    return values(m_tvdMslLogName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogFile::values(const QString& name) const
{
    CVF_ASSERT(m_wellLogFile);

    if (m_wellLogFile->HasContLog(name.toStdString()))
    {
        std::vector<double> logValues = m_wellLogFile->GetContLog(name.toStdString());
        
        for (size_t vIdx = 0; vIdx < logValues.size(); vIdx++)
        {
            if (m_wellLogFile->IsMissing(logValues[vIdx]))
            {
                // Convert missing ("NULL") values to HUGE_VAL
                logValues[vIdx] = HUGE_VAL;
            }
        }

        return logValues;
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
QString RigWellLogFile::wellLogChannelUnitString(const QString& wellLogChannelName, RiaDefines::DepthUnitType displayDepthUnit) const
{
    QString unit;

    NRLib::LasWell* lasWell = dynamic_cast<NRLib::LasWell*>(m_wellLogFile);
    if (lasWell)
    {
        unit = QString::fromStdString(lasWell->unitName(wellLogChannelName.toStdString()));
    }

    if (unit == depthUnitString())
    {
        if (displayDepthUnit != depthUnit())
        {
            if (displayDepthUnit == RiaDefines::UNIT_METER)
            {
                return "M";
            }
            else if (displayDepthUnit == RiaDefines::UNIT_FEET)
            {
                return "FT";
            }
            else if (displayDepthUnit == RiaDefines::UNIT_NONE)
            {
                CVF_ASSERT(false);
                return "";
            }
        }
    }

    return unit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellLogFile::hasTvdChannel() const
{
    return !m_tvdMslLogName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RigWellLogFile::depthUnit() const
{
    RiaDefines::DepthUnitType unitType = RiaDefines::UNIT_METER;

    if (depthUnitString().toUpper() == "F" || depthUnitString().toUpper() == "FT")
    {
        unitType = RiaDefines::UNIT_FEET;
    }

    return unitType;
}
