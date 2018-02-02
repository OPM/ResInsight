/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifEclipseSummaryTools.h"

#include "RiaSummaryCurveAnalyzer.h"
#include "RifReaderEclipseSummary.h"
#include "RiaStringEncodingTools.h"

#include "cafAppEnum.h"

#include "ert/ecl/ecl_util.h"

#include <QString>
#include <QStringList>

#include <iostream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryHeaderFile(const QString& inputFile, QString* headerFile, bool* isFormatted)
{
    findSummaryHeaderFileInfo(inputFile, headerFile, NULL, NULL, isFormatted);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryFiles(const QString& inputFile, 
                                                   QString* headerFile, 
                                                   QStringList* dataFiles)
{
    dataFiles->clear();
    headerFile->clear();

    char* myPath = NULL;
    char* myBase = NULL;
    char* myExtention = NULL;

    util_alloc_file_components(RiaStringEncodingTools::toNativeEncoded(inputFile).data(), &myPath, &myBase, &myExtention);

    QString path; if(myPath) path = RiaStringEncodingTools::fromNativeEncoded(myPath);
    QString base; if(myBase) base = RiaStringEncodingTools::fromNativeEncoded(myBase);
    std::string extention; if(myExtention) extention = myExtention;

    if(path.isEmpty() || base.isEmpty()) return ;

    char* myHeaderFile = NULL;
    stringlist_type* summary_file_list = stringlist_alloc_new();

    ecl_util_alloc_summary_files(RiaStringEncodingTools::toNativeEncoded(path).data(), RiaStringEncodingTools::toNativeEncoded(base).data(), extention.data(), &myHeaderFile, summary_file_list);
    if(myHeaderFile)
    {
        (*headerFile) = RiaStringEncodingTools::fromNativeEncoded(myHeaderFile);
        util_safe_free(myHeaderFile);
    }

    if(stringlist_get_size(summary_file_list) > 0)
    {
        for(int i = 0; i < stringlist_get_size(summary_file_list); i++)
        {
            dataFiles->push_back(RiaStringEncodingTools::fromNativeEncoded(stringlist_iget(summary_file_list,i)));
        }
    }
    stringlist_free(summary_file_list);

    return;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryTools::hasSummaryFiles(const QString& gridFileName)
{
    QString headerFileName;
    QStringList dataFileNames;
    RifEclipseSummaryTools::findSummaryFiles(gridFileName, &headerFileName, &dataFileNames);
    if (!headerFileName.isEmpty() && dataFileNames.size()) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseSummaryTools::findSummaryDataFiles(const QString& caseFile)
{
    QStringList fileNames;

    QString path;
    QString base;

    findSummaryHeaderFileInfo(caseFile, NULL, &path, &base, NULL);
    if (path.isEmpty() || base.isEmpty()) return fileNames;

    char* header_file = NULL;
    stringlist_type* summary_file_list = stringlist_alloc_new();

    ecl_util_alloc_summary_files(RiaStringEncodingTools::toNativeEncoded(path).data(), RiaStringEncodingTools::toNativeEncoded(base).data(), NULL, &header_file, summary_file_list);
    if (stringlist_get_size( summary_file_list ) > 0)
    {
        for (int i = 0; i < stringlist_get_size(summary_file_list); i++)
        {
            fileNames.push_back(RiaStringEncodingTools::fromNativeEncoded(stringlist_iget(summary_file_list, i)));
        }
    }

    util_safe_free(header_file);
    stringlist_free(summary_file_list);

    return fileNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::dumpMetaData(RifSummaryReaderInterface* readerEclipseSummary)
{
    std::vector<RifEclipseSummaryAddress> addresses = readerEclipseSummary->allResultAddresses();

    for (int category = 0; category < RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR; category++)
    {
        RifEclipseSummaryAddress::SummaryVarCategory categoryEnum = RifEclipseSummaryAddress::SummaryVarCategory(category);

        std::vector<RifEclipseSummaryAddress> catAddresses = RiaSummaryCurveAnalyzer::addressesForCategory(addresses, categoryEnum);

        if (catAddresses.size() > 0)
        {
            std::cout << caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText(categoryEnum).toStdString() << " count : " << catAddresses.size() << std::endl;

            for (size_t i = 0; i < catAddresses.size(); i++)
            {
                std::cout << catAddresses[i].quantityName() << " " 
                          << catAddresses[i].regionNumber() << " " 
                          << catAddresses[i].regionNumber2() << " "
                          << catAddresses[i].wellGroupName() << " "
                          << catAddresses[i].wellName() << " "
                          << catAddresses[i].wellSegmentNumber() << " "
                          << catAddresses[i].lgrName() << " "
                          << catAddresses[i].cellI() << " "
                          << catAddresses[i].cellJ() << " "
                          << catAddresses[i].cellK() << std::endl;
            }

            std::cout << std::endl;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryHeaderFileInfo(const QString& inputFile, QString* headerFile, QString* path, QString* base, bool* isFormatted)
{
    char* myPath = NULL;
    char* myBase = NULL;
    bool formattedFile = true;

    util_alloc_file_components(RiaStringEncodingTools::toNativeEncoded(inputFile).data(), &myPath, &myBase, NULL);

    char* myHeaderFile = ecl_util_alloc_exfilename(myPath, myBase, ECL_SUMMARY_HEADER_FILE, true, -1);
    if (!myHeaderFile)
    {
        myHeaderFile = ecl_util_alloc_exfilename(myPath, myBase, ECL_SUMMARY_HEADER_FILE, false, -1);
        if (myHeaderFile)
        {
            formattedFile = false;
        }
    }

    if (myHeaderFile && headerFile) *headerFile = myHeaderFile;
    if (myPath && path)             *path = myPath;
    if (myBase && base)             *base = myBase;
    if (isFormatted)                *isFormatted = formattedFile;

    util_safe_free(myHeaderFile);
    util_safe_free(myBase);
    util_safe_free(myPath);
}

