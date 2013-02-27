/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEclipseOutputFileTools.h"

#include <well_state.h>
#include <well_info.h>
#include <well_conn.h>
#include <well_ts.h>

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::RifEclipseUnifiedRestartFileAccess()
    : RifEclipseRestartDataAccess()
{
    m_ecl_file = NULL;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::~RifEclipseUnifiedRestartFileAccess()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// Open file
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::open(const QStringList& fileSet)
{
    m_filename = fileSet[0];

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::openFile()
{
    m_ecl_file = ecl_file_open(m_filename.toAscii().data());
    if (!m_ecl_file) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close file
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::close()
{
    if (m_ecl_file)
    {
        ecl_file_close(m_ecl_file);
    }

    m_ecl_file = NULL;
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUnifiedRestartFileAccess::timeStepCount()
{
    if (!openFile())
    {
        return 0;
    }

    return timeSteps().size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RifEclipseUnifiedRestartFileAccess::timeSteps()
{
    QList<QDateTime> timeSteps;

    if (openFile())
    {
        RifEclipseOutputFileTools::timeSteps(m_ecl_file, &timeSteps);
    }

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// Get list of result names
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts)
{
    if (openFile())
    {
        RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(m_ecl_file, resultNames, resultDataItemCounts);
    }
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values)
{
    if (!openFile())
    {
        return false;
    }

    size_t numOccurrences   = ecl_file_get_num_named_kw(m_ecl_file, resultName.toAscii().data());

    size_t startIndex       = timeStep * gridCount;
    CVF_ASSERT(startIndex + gridCount <= numOccurrences);

    size_t occurrenceIdx;
    for (occurrenceIdx = startIndex; occurrenceIdx < startIndex + gridCount; occurrenceIdx++)
    {
        std::vector<double> partValues;
        RifEclipseOutputFileTools::keywordData(m_ecl_file, resultName, occurrenceIdx, &partValues);

        values->insert(values->end(), partValues.begin(), partValues.end());
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::readWellData(well_info_type* well_info)
{
    if (!well_info) return;

    if (openFile())
    {
        well_info_add_UNRST_wells(well_info, m_ecl_file);
    }
}

