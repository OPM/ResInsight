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

#include "RifEclipseOutputFileTools.h"

#include "util.h"
#include "ecl_file.h"
#include "ecl_kw_magic.h"
#include "ecl_grid.h"

#include <QFileInfo>
#include <QDebug>
#include "cafProgressInfo.h"


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::RifEclipseOutputFileTools()
{
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::~RifEclipseOutputFileTools()
{
}

//--------------------------------------------------------------------------------------------------
/// Get list of time step texts (dates)
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::timeSteps(ecl_file_type* ecl_file, std::vector<QDateTime>* timeSteps, bool* detectedFractionOfDay )
{
    CVF_ASSERT(timeSteps);
    CVF_ASSERT(ecl_file);

    // Get the number of occurrences of the INTEHEAD keyword
    int numINTEHEAD = ecl_file_get_num_named_kw(ecl_file, INTEHEAD_KW);
    
    // Get the number of occurrences of the DOUBHEAD keyword
    int numDOUBHEAD = ecl_file_get_num_named_kw(ecl_file, DOUBHEAD_KW);

    CVF_ASSERT(numINTEHEAD == numDOUBHEAD);

    bool hasFractionOfDay = false;
    bool foundAllDayValues = false;
    const double delta = 0.001;

    // Find all days, and stop when the double value is lower than the previous
    QList<double> days;
    for (int i = 0; i < numDOUBHEAD; i++)
    {
        if (foundAllDayValues) continue;;

        ecl_kw_type* kwDOUBHEAD = ecl_file_iget_named_kw(ecl_file, DOUBHEAD_KW, i);
        if (kwDOUBHEAD)
        {
            double dayValue = ecl_kw_iget_double(kwDOUBHEAD, DOUBHEAD_DAYS_INDEX);
            double floorDayValue = cvf::Math::floor(dayValue);

            if (dayValue - floorDayValue > delta)
            {
                hasFractionOfDay = true;
            }

            days.push_back(dayValue);
        }
    }

    std::vector<QDateTime> timeStepsFound;
   
    if (hasFractionOfDay)
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(ecl_file, INTEHEAD_KW, 0);
        if (kwINTEHEAD)
        {
            int day     = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_DAY_INDEX);
            int month   = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_MONTH_INDEX);
            int year    = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_YEAR_INDEX);
            QDate simulationStart(year, month, day);

            for (int i = 0; i < days.size(); i++)
            {
                double dayValue = days[i];
                double floorDayValue = cvf::Math::floor(dayValue);
                double dayFraction = dayValue - floorDayValue;

                int seconds = static_cast<int>(dayFraction * 24.0 * 60.0 * 60.0);
                QTime time(0, 0);
                time = time.addSecs(seconds);

                QDate reportDate = simulationStart;
                reportDate = reportDate.addDays(static_cast<int>(floorDayValue));

                QDateTime reportDateTime(reportDate, time);
                if (std::find(timeStepsFound.begin(), timeStepsFound.end(), reportDateTime) ==  timeStepsFound.end())
                {
                    timeStepsFound.push_back(reportDateTime);
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < numINTEHEAD; i++)
        {
            ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(ecl_file, INTEHEAD_KW, i);
            if (kwINTEHEAD)
            {
                int day     = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_DAY_INDEX);
                int month   = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_MONTH_INDEX);
                int year    = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_YEAR_INDEX);

                QDate reportDate(year, month, day);
                CVF_ASSERT(reportDate.isValid());
                
                QDateTime reportDateTime(reportDate);
                if (std::find(timeStepsFound.begin(), timeStepsFound.end(), reportDateTime) ==  timeStepsFound.end())
                {
                    timeStepsFound.push_back(reportDateTime);
                }
            }
        }
    }

    // Return time step info to caller
    *timeSteps = timeStepsFound;

    if (detectedFractionOfDay)
    {
        *detectedFractionOfDay = hasFractionOfDay;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordData(ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<double>* values)
{
    ecl_kw_type* kwData = ecl_file_iget_named_kw(ecl_file, keyword.toAscii().data(), static_cast<int>(fileKeywordOccurrence));
    if (kwData)
    {
        size_t numValues = ecl_kw_get_size(kwData);

        std::vector<double> doubleData;
        doubleData.resize(numValues);

        ecl_kw_get_data_as_double(kwData, doubleData.data());
        values->insert(values->end(), doubleData.begin(), doubleData.end());

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordData(ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<int>* values)
{
    ecl_kw_type* kwData = ecl_file_iget_named_kw(ecl_file, keyword.toAscii().data(), static_cast<int>(fileKeywordOccurrence));
    if (kwData)
    {
        size_t numValues = ecl_kw_get_size(kwData);

        std::vector<int> integerData;
        integerData.resize(numValues);

        ecl_kw_get_memcpy_int_data(kwData, integerData.data());
        values->insert(values->end(), integerData.begin(), integerData.end());

        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Get first occurrence of file of given type in given list of filenames, as filename or NULL if not found
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputFileTools::fileNameByType(const QStringList& fileSet, ecl_file_enum fileType)
{
    int i;
    for (i = 0; i < fileSet.count(); i++)
    {
        bool formatted = false;
        int reportNumber = -1;
        if (ecl_util_get_file_type(fileSet.at(i).toAscii().data(), &formatted, &reportNumber) == fileType)
        {
            return fileSet.at(i);
        }
    }

    return QString::null;
}


//--------------------------------------------------------------------------------------------------
/// Get all files of file of given type in given list of filenames, as filename or NULL if not found
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseOutputFileTools::fileNamesByType(const QStringList& fileSet, ecl_file_enum fileType)
{
    QStringList fileNames;

    int i;
    for (i = 0; i < fileSet.count(); i++)
    {
        bool formatted = false;
        int reportNumber = -1;
        if (ecl_util_get_file_type(fileSet.at(i).toAscii().data(), &formatted, &reportNumber) == fileType)
        {
            fileNames.append(fileSet.at(i));
        }
    }

    return fileNames;
}


//--------------------------------------------------------------------------------------------------
/// Get set of Eclipse files based on an input file and its path
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::fileSet(const QString& fileName, QStringList* fileSet)
{
    CVF_ASSERT(fileSet);
    fileSet->clear();

    QString filePath = QFileInfo(fileName).absoluteFilePath();
    filePath = QFileInfo(filePath).path();
    QString fileNameBase = QFileInfo(fileName).baseName();

    stringlist_type* eclipseFiles = stringlist_alloc_new();
    ecl_util_select_filelist(filePath.toAscii().data(), fileNameBase.toAscii().data(), ECL_OTHER_FILE, false, eclipseFiles);

    int i;
    for (i = 0; i < stringlist_get_size(eclipseFiles); i++)
    {
        fileSet->append(stringlist_safe_iget(eclipseFiles, i));
    }

    stringlist_free(eclipseFiles);

    return fileSet->count() > 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::findKeywordsAndDataItemCounts(ecl_file_type* ecl_file, QStringList* keywords, std::vector<size_t>* keywordDataItemCounts)
{
    if (!ecl_file || !keywords || !keywordDataItemCounts) return;

    int numKeywords = ecl_file_get_num_distinct_kw(ecl_file);

    caf::ProgressInfo info(numKeywords, "Reading Keywords on file");

    for (int i = 0; i < numKeywords; i++)
    {
        const char* kw = ecl_file_iget_distinct_kw(ecl_file , i);
        int numKeywordOccurrences = ecl_file_get_num_named_kw(ecl_file, kw);
        bool validData = true;
        size_t fileResultValueCount = 0;
        for (int j = 0; j < numKeywordOccurrences; j++)
        {
            fileResultValueCount += ecl_file_iget_named_size(ecl_file, kw, j);

            ecl_type_enum dataType = ecl_file_iget_named_type(ecl_file, kw, j);
            if (dataType != ECL_DOUBLE_TYPE && dataType != ECL_FLOAT_TYPE && dataType != ECL_INT_TYPE )
            {
                validData = false;
                break;
            }
        }

        if (validData)
        {
            keywords->append(QString(kw));
            keywordDataItemCounts->push_back(fileResultValueCount);
        }

        info.setProgress(i);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::readGridDimensions(const QString& gridFileName, std::vector< std::vector<int> >& gridDimensions)
{
    int gridDims[3];

    bool ret = ecl_grid_file_dims(gridFileName.toAscii().data(), NULL, gridDims);
    if (ret)
    {
        gridDimensions.resize(1);
        gridDimensions[0].push_back(gridDims[0]);
        gridDimensions[0].push_back(gridDims[1]);
        gridDimensions[0].push_back(gridDims[2]);
    }
}
