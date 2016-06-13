/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_grid.h"
#include "ert/ecl/ecl_kw_magic.h"

#include "cafProgressInfo.h"

#include <QFileInfo>
#include <QDebug>

#include <assert.h>


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

void getDayMonthYear(const ecl_kw_type* intehead_kw, int* day, int* month, int* year)
{
    assert(day && month && year);

    *day = ecl_kw_iget_int(intehead_kw, INTEHEAD_DAY_INDEX);
    *month = ecl_kw_iget_int(intehead_kw, INTEHEAD_MONTH_INDEX);
    *year = ecl_kw_iget_int(intehead_kw, INTEHEAD_YEAR_INDEX);
}

//--------------------------------------------------------------------------------------------------
/// Get list of time step texts (dates)
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::timeSteps(ecl_file_type* ecl_file, std::vector<QDateTime>* timeSteps)
{
    CVF_ASSERT(timeSteps);
    CVF_ASSERT(ecl_file);

    // Get the number of occurrences of the INTEHEAD keyword
    int numINTEHEAD = ecl_file_get_num_named_kw(ecl_file, INTEHEAD_KW);
    
    // Get the number of occurrences of the DOUBHEAD keyword
    int numDOUBHEAD = ecl_file_get_num_named_kw(ecl_file, DOUBHEAD_KW);

    std::vector<double> dayFractions(numINTEHEAD, 0.0); // Init fraction to zero

    // Read out fraction of day if number of keywords are identical
    if (numINTEHEAD == numDOUBHEAD)
    {
        for (int i = 0; i < numDOUBHEAD; i++)
        {
            ecl_kw_type* kwDOUBHEAD = ecl_file_iget_named_kw(ecl_file, DOUBHEAD_KW, i);
            if (kwDOUBHEAD)
            {
                double dayValue = ecl_kw_iget_double(kwDOUBHEAD, DOUBHEAD_DAYS_INDEX);
                double floorDayValue = cvf::Math::floor(dayValue);

                double dayDelta = dayValue - floorDayValue;

                dayFractions[i] = dayDelta;
            }
        }
    }

    for (int i = 0; i < numINTEHEAD; i++)
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(ecl_file, INTEHEAD_KW, i);
        CVF_ASSERT(kwINTEHEAD);
        if (kwINTEHEAD)
        {
            int day = 0;
            int month = 0;
            int year = 0;
            getDayMonthYear(kwINTEHEAD, &day, &month, &year);
            
            QDateTime reportDateTime(QDate(year, month, day));
            CVF_ASSERT(reportDateTime.isValid());

            double dayFraction = dayFractions[i];
            int seconds = static_cast<int>(dayFraction * 24.0 * 60.0 * 60.0);
            QTime time(0, 0);
            time = time.addSecs(seconds);

            reportDateTime.setTime(time);

            if (std::find(timeSteps->begin(), timeSteps->end(), reportDateTime) == timeSteps->end())
            {
                timeSteps->push_back(reportDateTime);
            }
        }
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
QString RifEclipseOutputFileTools::firstFileNameOfType(const QStringList& fileSet, ecl_file_enum fileType)
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
/// Get all files of the given type from the provided list of filenames 
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseOutputFileTools::filterFileNamesOfType(const QStringList& fileSet, ecl_file_enum fileType)
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
bool RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(const QString& fullPathFileName, QStringList* baseNameFiles)
{
    CVF_ASSERT(baseNameFiles);
    baseNameFiles->clear();

    QString filePath = QFileInfo(fullPathFileName).absoluteFilePath();
    filePath = QFileInfo(filePath).path();
    QString fileNameBase = QFileInfo(fullPathFileName).baseName();

    stringlist_type* eclipseFiles = stringlist_alloc_new();
    ecl_util_select_filelist(filePath.toAscii().data(), fileNameBase.toAscii().data(), ECL_OTHER_FILE, false, eclipseFiles);

    int i;
    for (i = 0; i < stringlist_get_size(eclipseFiles); i++)
    {
        baseNameFiles->append(stringlist_safe_iget(eclipseFiles, i));
    }

    stringlist_free(eclipseFiles);

    return baseNameFiles->count() > 0;
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
    ecl_grid_type * grid         = ecl_grid_alloc(gridFileName.toAscii().data());                               // bootstrap ecl_grid instance
    stringlist_type * lgr_names  = ecl_grid_alloc_lgr_name_list( grid );                                   // get a list of all the lgr names.

    //printf("grid:%s has %d a total of %d lgr's \n", grid_filename , stringlist_get_size( lgr_names ));
    for (int lgr_nr = 0; lgr_nr < stringlist_get_size( lgr_names); lgr_nr++)
    {
        ecl_grid_type * lgr_grid  = ecl_grid_get_lgr( grid , stringlist_iget( lgr_names , lgr_nr ));    // get the ecl_grid instance of the lgr - by name.

        int nx,ny,nz,active_size;
        ecl_grid_get_dims( lgr_grid , &nx , &ny , &nz , &active_size);                             // get some size info from this lgr.

        std::vector<int> values;
        values.push_back(nx);
        values.push_back(ny);
        values.push_back(nz);
        values.push_back(active_size);

        gridDimensions.push_back(values);
    }

    ecl_grid_free( grid );
    stringlist_free( lgr_names );

}

//--------------------------------------------------------------------------------------------------
/// Returns the following integer values from the first INTEHEAD keyword found
///  1  : METRIC
///  2  : FIELD
///  3  : LAB
///  -1 : No INTEHEAD keyword found
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputFileTools::readUnitsType(ecl_file_type* ecl_file)
{
    int unitsType = -1;

    if (ecl_file)
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw(ecl_file, INTEHEAD_KW, 0);
        if (kwINTEHEAD)
        {
            unitsType = ecl_kw_iget_int(kwINTEHEAD, INTEHEAD_UNIT_INDEX);
        }
    }

    return unitsType;
}
