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
bool RifEclipseRestartFilesetAccess::open(const QStringList& fileSet, const std::vector<size_t>& matrixActiveCellCounts, const std::vector<size_t>& fractureActiveCellCounts)
{
    close();

    int numFiles = fileSet.size();
    int i;
    for (i = 0; i < numFiles; i++)
    {
        cvf::ref<RifEclipseOutputFileTools> fileAccess = new RifEclipseOutputFileTools;
        if (!fileAccess->open(fileSet[i], matrixActiveCellCounts, fractureActiveCellCounts))
        {
            close();
            return false;
        }

        m_files.push_back(fileAccess);
    }

    m_gridCount = matrixActiveCellCounts.size();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close files
//--------------------------------------------------------------------------------------------------
void RifEclipseRestartFilesetAccess::close()
{
    m_files.clear();
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseRestartFilesetAccess::numTimeSteps()
{
    return m_files.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time step texts
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseRestartFilesetAccess::timeStepsText()
{
    QStringList timeSteps;

    size_t numSteps = numTimeSteps();
    size_t i;
    for (i = 0; i < numSteps; i++)
    {
        QStringList stepText;
        m_files[i]->timeStepsText(&stepText);
        timeSteps.append(stepText.size() == 1 ? stepText : QStringList(QString("Step %1").arg(i+1)));
    }

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RifEclipseRestartFilesetAccess::timeSteps()
{
    QList<QDateTime> timeSteps;

    size_t numSteps = numTimeSteps();
    size_t i;
    for (i = 0; i < numSteps; i++)
    {
        QList<QDateTime> stepTime;
        m_files[i]->timeSteps(&stepTime);
        
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
QStringList RifEclipseRestartFilesetAccess::resultNames()
{
    CVF_ASSERT(numTimeSteps() > 0);

    // Get the results found on the first file
    QStringList resultsList;
    m_files[0]->validKeywords(&resultsList, RifReaderInterface::MATRIX_RESULTS);

    return resultsList;
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseRestartFilesetAccess::results(const QString& resultName, RifReaderInterface::PorosityModelResultType matrixOrFracture, size_t timeStep, std::vector<double>* values)
{
    size_t numOccurrences = m_files[timeStep]->numOccurrences(resultName);

    // No results for this result variable for current time step found
    if (numOccurrences == 0) return true;

    // Result handling depends on presents of result values for all grids
    if (m_gridCount != numOccurrences)
    {
        return false;
    }

    size_t i;
    for (i = 0; i < numOccurrences; i++)
    {
        std::vector<double> partValues;
        if (!m_files[timeStep]->keywordData(resultName, i, matrixOrFracture, &partValues))  // !! don't need to append afterwards
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

    for (size_t i = 0; i < m_files.size(); i++)
    {
        const char* fileName = ecl_file_get_src_file(m_files[i]->filePointer());
        int reportNumber = ecl_util_filename_report_nr(fileName);
        if(reportNumber != -1)
        {
            well_info_add_wells(well_info, m_files[i]->filePointer(), reportNumber);
        }
    }
}
