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
#include "ecl_intehead.h"

#include <QFileInfo>
#include "cafProgressInfo.h"

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::RifEclipseOutputFileTools()
{
    m_file = NULL;
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::~RifEclipseOutputFileTools()
{
    close();
}


//--------------------------------------------------------------------------------------------------
/// Open file given by name
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::open(const QString& fileName)
{
    // Close current file if any
    close();

    m_file = ecl_file_open(fileName.toAscii().data());
    if (!m_file) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Close file
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::close()
{
    if (m_file)
    {
        ecl_file_close(m_file);
        m_file = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Get the number of occurrences of the given keyword
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputFileTools::numOccurrences(const QString& keyword)
{
    CVF_ASSERT(m_file);
    return ecl_file_get_num_named_kw(m_file, keyword.toAscii().data());
}


//--------------------------------------------------------------------------------------------------
/// Get keywords found on file given by name.
/// If numDataItems != cvf::UNDEFINED_SIZE_T, get keywords with that exact number of data items only.
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordsOnFile(QStringList* keywords, size_t numDataItems, size_t numSteps)
{
    CVF_ASSERT(m_file);
    CVF_ASSERT(keywords);
    keywords->clear();

    int numKeywords = ecl_file_get_num_distinct_kw(m_file);

    caf::ProgressInfo info(numKeywords, "Reading Keywords on file");

    for (int i = 0; i < numKeywords; i++)
    {
        const char* kw = ecl_file_iget_distinct_kw(m_file , i);
        int numKWOccurences = ecl_file_get_num_named_kw(m_file, kw);

        if (numDataItems != cvf::UNDEFINED_SIZE_T)
        {
            bool dataTypeSupported = true;
            int numKWValues = 0;
            int j;
            for (j = 0; j < numKWOccurences; j++)
            {
                numKWValues += ecl_file_iget_named_size(m_file, kw, j);

                // Check the data type - only float and double are supported
                ecl_type_enum dataType = ecl_file_iget_named_type(m_file, kw, j);
                if (dataType != ECL_DOUBLE_TYPE && dataType != ECL_FLOAT_TYPE && dataType != ECL_INT_TYPE )
                {
                    dataTypeSupported = false;
                    break;
                }
            }

            if (dataTypeSupported)
            {
                if (numSteps != cvf::UNDEFINED_SIZE_T && numSteps > 0)
                {
                    numKWValues /= static_cast<int>(numSteps);
                }

                // Append keyword to the list if it has the given number of values in total
                if (numKWValues == static_cast<int>(numDataItems))
                {
                    keywords->append(QString(kw));
                }
            }
        }
        else
        {
            keywords->append(QString(kw));
        }

        info.setProgress(i);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Get list of time step texts (dates)
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::timeStepsText(QStringList* timeSteps)
{
    CVF_ASSERT(timeSteps);
    CVF_ASSERT(m_file);

    const char* KW_INTEHEAD = "INTEHEAD";

    // Get the number of occurrences of the INTEHEAD keyword
    int numINTEHEAD = numOccurrences(KW_INTEHEAD);

    QStringList timeStepsFound;
    int i;
    for (i = 0; i < numINTEHEAD; i++)
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(m_file, KW_INTEHEAD, i);
        if (kwINTEHEAD)
        {
            // Get date info
            time_t stepTime = ecl_intehead_date(kwINTEHEAD);

            // Hack!!! We seem to get 01/01/1970 (time -1) for sub grids!
            if (stepTime < 0) continue;

            // Build date string
            char* dateString = util_alloc_date_string(stepTime);
            timeStepsFound += QString(dateString);
            util_safe_free(dateString);
        }
    }

    // Time steps are given for both the main grid and all sub grids,
    // so we need to make sure that duplicates are removed
    timeStepsFound.removeDuplicates();

    // Return time step info to caller
    *timeSteps = timeStepsFound;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Get list of time step texts (dates)
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::timeSteps(QList<QDateTime>* timeSteps)
{
    CVF_ASSERT(timeSteps);
    CVF_ASSERT(m_file);

    const char* KW_INTEHEAD = "INTEHEAD";

    // Get the number of occurrences of the INTEHEAD keyword
    int numINTEHEAD = numOccurrences(KW_INTEHEAD);

    QList<QDateTime> timeStepsFound;
    int i;
    for (i = 0; i < numINTEHEAD; i++)
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(m_file, KW_INTEHEAD, i);
        if (kwINTEHEAD)
        {
            // Get date info
            time_t stepTime = ecl_intehead_date(kwINTEHEAD);

            // Hack!!! We seem to get 01/01/1970 (time -1) for sub grids!
            if (stepTime < 0) continue;

            // Build date string
            QDateTime dateTime = QDateTime::fromTime_t(stepTime);

            if (timeStepsFound.indexOf(dateTime) < 0)
            {
                timeStepsFound.push_back(dateTime);
            }
        }
    }

    // Return time step info to caller
    *timeSteps = timeStepsFound;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Get first occurrence of file of given type in given list of filenames, as filename or NULL if not found
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordData(const QString& keyword, size_t index, std::vector<double>* values)
{
    CVF_ASSERT(m_file);
    CVF_ASSERT(values);

    ecl_kw_type* kwData = ecl_file_iget_named_kw(m_file, keyword.toAscii().data(), static_cast<int>(index));
    if (kwData)
    {
        size_t numValues = ecl_kw_get_size(kwData);

        std::vector<double> doubleData;
        doubleData.resize(numValues);

        ecl_kw_get_data_as_double(kwData, doubleData.data());
        values->insert(values->end(), doubleData.begin(), doubleData.end());
    }

    return true;
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
ecl_file_type* RifEclipseOutputFileTools::filePointer()
{
    return m_file;
}
