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

#include "RifReaderEclipseRestartFiles.h"
#include "RifReaderEclipseFileAccess.h"


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRestartFiles::RifReaderEclipseRestartFiles(size_t numGrids, size_t numActiveCells)
    : RifReaderEclipseResultsAccess(numGrids, numActiveCells)
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRestartFiles::~RifReaderEclipseRestartFiles()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// Open files
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseRestartFiles::open(const QStringList& fileSet)
{
    close();

    size_t numFiles = fileSet.size();
    size_t i;
    for (i = 0; i < numFiles; i++)
    {
        cvf::ref<RifReaderEclipseFileAccess> fileAccess = new RifReaderEclipseFileAccess;
        if (!fileAccess->open(fileSet[i]))
        {
            close();
            return false;
        }

        m_files.push_back(fileAccess);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close files
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRestartFiles::close()
{
    m_files.clear();
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifReaderEclipseRestartFiles::numTimeSteps()
{
    return m_files.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time step texts
//--------------------------------------------------------------------------------------------------
QStringList RifReaderEclipseRestartFiles::timeStepsText()
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
QList<QDateTime> RifReaderEclipseRestartFiles::timeSteps()
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
QStringList RifReaderEclipseRestartFiles::resultNames()
{
    CVF_ASSERT(numTimeSteps() > 0);

    // Get the results found on the first file
    QStringList resultsList;
    m_files[0]->keywordsOnFile(&resultsList, m_numActiveCells, 1);

    return resultsList;
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseRestartFiles::results(const QString& resultName, size_t timeStep, std::vector<double>* values)
{
    size_t numOccurrences = m_files[timeStep]->numOccurrences(resultName);
    CVF_ASSERT(m_numGrids == numOccurrences);

    size_t i;
    for (i = 0; i < numOccurrences; i++)
    {
        std::vector<double> partValues;
        if (!m_files[timeStep]->keywordData(resultName, i, &partValues))  // !! don't need to append afterwards
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
#ifdef USE_ECL_LIB
void RifReaderEclipseRestartFiles::readWellData(well_info_type* well_info)
{
    if (!well_info) return;

    size_t i;
    for (i=0; i < m_files.size(); i++)
    {
        well_info_add_UNRST_wells(well_info, m_files[i]->filePointer());
    }
}
#endif
