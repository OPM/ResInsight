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

#include "RifEclipseRestartFilesetAccess.h"
#include "RifEclipseOutputFileTools.h"
#include "cafProgressInfo.h"


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseRestartFilesetAccess::RifEclipseRestartFilesetAccess()
    : RifEclipseRestartDataAccess()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseRestartFilesetAccess::~RifEclipseRestartFilesetAccess()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// Open files
//--------------------------------------------------------------------------------------------------
bool RifEclipseRestartFilesetAccess::open(const QStringList& fileSet)
{
    close();

    int numFiles = fileSet.size();

    caf::ProgressInfo progInfo(numFiles,"");

    int i;
    for (i = 0; i < numFiles; i++)
    {
        progInfo.setProgressDescription(fileSet[i]);

        ecl_file_type* ecl_file = ecl_file_open(fileSet[i].toAscii().data());
        if (!ecl_file) return false;

        m_ecl_files.push_back(ecl_file);

        progInfo.incrementProgress();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close files
//--------------------------------------------------------------------------------------------------
void RifEclipseRestartFilesetAccess::close()
{
    for (size_t i = 0; i < m_ecl_files.size(); i++)
    {
        ecl_file_close(m_ecl_files[i]);
    }
    m_ecl_files.clear();

}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseRestartFilesetAccess::timeStepCount()
{
    return m_ecl_files.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RifEclipseRestartFilesetAccess::timeSteps()
{
    QList<QDateTime> timeSteps;

    size_t numSteps = timeStepCount();
    size_t i;
    for (i = 0; i < numSteps; i++)
    {
        QList<QDateTime> stepTime;

        RifEclipseOutputFileTools::timeSteps(m_ecl_files[i], &stepTime);
        
        if (stepTime.size() == 1)
        {
            timeSteps.push_back(stepTime[0]);
        }
        else
        {
            timeSteps.push_back(QDateTime());
        }
    }

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// Get list of result names
//--------------------------------------------------------------------------------------------------
void RifEclipseRestartFilesetAccess::resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts)
{
    CVF_ASSERT(timeStepCount() > 0);

    std::vector<size_t> valueCountForOneFile;
    RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(m_ecl_files[0], resultNames, &valueCountForOneFile);

    for (size_t i = 0; i < valueCountForOneFile.size(); i++)
    {
        resultDataItemCounts->push_back(valueCountForOneFile[i] * timeStepCount());
    }
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseRestartFilesetAccess::results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values)
{
    size_t numOccurrences = ecl_file_get_num_named_kw(m_ecl_files[timeStep], resultName.toAscii().data());

    // No results for this result variable for current time step found
    if (numOccurrences == 0) return true;

    // Result handling depends on presents of result values for all grids
    if (gridCount != numOccurrences)
    {
        return false;
    }

    size_t i;
    for (i = 0; i < numOccurrences; i++)
    {
        std::vector<double> partValues;

        if (!RifEclipseOutputFileTools::keywordData(m_ecl_files[timeStep], resultName, i, &partValues))
        {
            return false;
        }

        values->insert(values->end(), partValues.begin(), partValues.end());
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseRestartFilesetAccess::readWellData(well_info_type* well_info)
{
    if (!well_info) return;

    for (size_t i = 0; i < m_ecl_files.size(); i++)
    {
        const char* fileName = ecl_file_get_src_file(m_ecl_files[i]);
        int reportNumber = ecl_util_filename_report_nr(fileName);
        if(reportNumber != -1)
        {
            well_info_add_wells(well_info, m_ecl_files[i], reportNumber);
        }
    }
}
