/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C)  Statoil ASA
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

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseSummary.h"

#include <QDateTime>
#include <QDir>

#include "RifSummaryReaderMultipleFiles.h"
#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifEclipseSummaryTest, TestRestartSummaryFileReferences_01 )
{
    QString summaryFileName =
        "d:/Dropbox/Dropbox (Ceetron Solutions)/Projects/20084 ResInsight Introduction and Advanced "
        "courses/intro2020_data/reek_ensemble/3_r001_reek_50/realization-0/base_pred/eclipse/model/"
        "3_R001_REEK-0.SMSPEC";

    std::vector<QString>            warnings;
    std::vector<RifRestartFileInfo> originFileInfos = RifEclipseSummaryTools::getRestartFiles( summaryFileName, warnings );
    EXPECT_TRUE( originFileInfos.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifEclipseSummaryTest, TestRestartSummaryFileReferences_02 )
{
    QString summaryFileName =
        "e:/models/reek_ensemble/3_r001_reek_50/realization-0/base_pred/eclipse/model/3_R001_REEK-0.SMSPEC";

    std::vector<QString>            warnings;
    std::vector<RifRestartFileInfo> originFileInfos = RifEclipseSummaryTools::getRestartFiles( summaryFileName, warnings );

    if ( !originFileInfos.empty() )
    {
        std::vector<std::string> smspecFilesNewFirst;
        smspecFilesNewFirst.push_back( summaryFileName.toStdString() );
        for ( const auto& s : originFileInfos )
        {
            smspecFilesNewFirst.push_back( s.fileName.toStdString() );
        }

        RifSummaryReaderMultipleFiles multipleSummaryFiles( smspecFilesNewFirst );
        auto                          ts = multipleSummaryFiles.timeSteps( {} );
        std::cout << ts.size();
    }

    EXPECT_TRUE( originFileInfos.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifEclipseSummaryTest, BasicTestSetCurrentFolder )
{
    static const QString testDataRootFolder = QString( "%1/SummaryData/Reek/" ).arg( TEST_DATA_DIR );

    QString summaryFileName = testDataRootFolder + "3_R001_REEK-1.SMSPEC";

    std::vector<QString>            warnings;
    std::vector<RifRestartFileInfo> originFileInfos = RifEclipseSummaryTools::getRestartFiles( summaryFileName, warnings );
    EXPECT_TRUE( originFileInfos.empty() );
}

/*
void printDateAndValues(const std::vector<QDateTime>& dates, const std::vector<double>& values)
{
    EXPECT_TRUE(dates.size() == values.size());

    for (size_t i = 0; i < values.size(); i++)
    {
        std::string dateStr = dates[i].toString("dd/MMM/yyyy").toStdString();

        std::cout << dateStr << " " <<  values[i] << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, SummaryToolsFindSummaryFiles)
{
    {
//        std::string filename = "g:/\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010";
        std::string filename = "g:\\Models\\Statoil\\testcase_juli_2011\\data\\TEST10K_FLT_LGR_NNC";

        {
            std::string headerFile;
            bool isFormatted = false;
            RifEclipseSummaryTools::findSummaryHeaderFile(filename, &headerFile, &isFormatted);

            EXPECT_FALSE(isFormatted);
            EXPECT_FALSE(headerFile.empty());

            std::vector<std::string> dataFiles = RifEclipseSummaryTools::findSummaryDataFiles(filename);
            EXPECT_TRUE(dataFiles.size() > 0);

            std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary); eclSummary->open(headerFile, dataFiles);

            RifEclipseSummaryTools::dumpMetaData(eclSummary.get());

            // Create a vector of summary addresses based on type, item name and variable name, and compare the
resulting
            // resultAddressString to the original string

            std::vector<RifEclipseSummaryAddress> addresses = eclSummary->allResultAddresses();
            std::vector<RifEclipseSummaryAddress> myAddresses;
            for (size_t i = 0; i < addresses.size(); i++)
            {
                RifEclipseSummaryAddress adr(addresses[i].category(), addresses[i].simulationItemName(),
addresses[i].quantityName()); myAddresses.push_back(adr);
            }

            for (size_t i = 0; i < addresses.size(); i++)
            {
                EXPECT_TRUE(addresses[i].ertSummaryVarId().compare(myAddresses[i].ertSummaryVarId()) == 0);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, SummaryToolsFindSummaryFiles)
{
    {
        std::string filename = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010";

        {
            std::string headerFile;
            bool isFormatted = false;
            RifEclipseSummaryTools::findSummaryHeaderFile(filename, &headerFile, &isFormatted);

            EXPECT_FALSE(isFormatted);
            EXPECT_FALSE(headerFile.empty());

            std::vector<std::string> dataFiles = RifEclipseSummaryTools::findSummaryDataFiles(filename);
            EXPECT_TRUE(dataFiles.size() > 0);
        }
    }

    {
        std::string filename = "g:\\Models\\Statoil\\testcase_juli_2011\\data\\TEST10K_FLT_LGR_NNC";

        {
            std::string headerFile;
            bool isFormatted = false;
            RifEclipseSummaryTools::findSummaryHeaderFile(filename, &headerFile, &isFormatted);

            EXPECT_FALSE(isFormatted);
            EXPECT_FALSE(headerFile.empty());

            std::vector<std::string> dataFiles = RifEclipseSummaryTools::findSummaryDataFiles(filename);
            EXPECT_TRUE(dataFiles.size() > 0);

            std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary); eclSummary->open(headerFile, dataFiles);

            RifEclipseSummaryTools::dumpMetaData(eclSummary.get());
        }
    }








    {
        // MSJ TODO: Formatted output does not work now, should be reported?
/ *
        std::string filename = "g:\\Models\\Statoil\\Brillig\\BRILLIG_FMTOUT";

        {
            std::string headerFile;
            bool isFormatted = false;
            RifEclipseSummaryTools::findSummaryHeaderFile(filename, &headerFile, &isFormatted);

            EXPECT_FALSE(isFormatted);
            EXPECT_FALSE(headerFile.empty());

            std::vector<std::string> dataFiles = RifEclipseSummaryTools::findSummaryDataFiles(filename);
            EXPECT_TRUE(dataFiles.size() > 0);

            std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary); eclSummary->open(headerFile, dataFiles);

            RifEclipseSummaryTools::dumpMetaData(eclSummary.get());
        }
* /
    }








/ *
    {
        std::string path;
        std::string base;
        bool isFormatted = false;
        RifEclipseSummaryTools::findSummaryHeaderFile(filename, &path, &base, &isFormatted);

    }
* /

}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, BasicReadKeywordTest)
{
    std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary);








    std::string filename = "g:\\Models\\Statoil\\testcase_juli_2011\\data\\TEST10K_FLT_LGR_NNC";

    std::string headerFileName;
    RifEclipseSummaryTools::findSummaryHeaderFile(filename, &headerFileName, NULL);

    std::vector<std::string> dataFileNames = RifEclipseSummaryTools::findSummaryDataFiles(filename);








    eclSummary->open(headerFileName, dataFileNames);

    RifEclipseSummaryTools::dumpMetaData(eclSummary.get());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, DISABLE_BasicReadKeywordTest)
{
    std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary);








    std::string filename = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.SMSPEC";
    std::vector<std::string> dataFileNames;
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0001");
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0002");
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0003");
    eclSummary->open(filename, dataFileNames);

    std::cout << " -- Well names --" << std::endl;
    {
        std::vector<std::string> names =  eclSummary->wellNames();

        for (size_t i = 0; i < names.size(); i++)
        {
            std::cout << names[i] << std::endl;
        }
    }

    std::cout << " -- Well variable names --" << std::endl;
    {
        std::vector<std::string> names =  eclSummary->wellVariableNames();

        for (size_t i = 0; i < names.size(); i++)
        {
            std::cout << names[i] << std::endl;
        }
    }

    std::cout << " -- Group names --" << std::endl;
    {
        std::vector<std::string> names =  eclSummary->wellGroupNames();

        for (size_t i = 0; i < names.size(); i++)
        {
            std::cout << names[i] << std::endl;
        }
    }

}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, BasicReadDataTest)
{
    std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary);








    std::string filename = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.SMSPEC";
    std::vector<std::string> dataFileNames;
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0001");
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0012");
    dataFileNames.push_back("g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010.S0023");
    eclSummary->open(filename, dataFileNames);

    std::vector<QDateTime> dateTimes;
    {
        std::vector<time_t> times = eclSummary->timeSteps();
        dateTimes = RifReaderEclipseSummary::fromTimeT(times);
    }

    {
        std::string keyword = "YEARS";
        std::cout << std::endl << keyword << std::endl;

        std::vector<double> values;
        eclSummary->values(keyword, &values);
        printDateAndValues(dateTimes, values);
    }

    {
        std::string keyword = "WWPR:P20";
        std::cout << std::endl << keyword << std::endl;

        std::vector<double> values;
        eclSummary->values(keyword, &values);
        printDateAndValues(dateTimes, values);
    }









}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, DISABLED_StringlistSelectMatchingFiles)
{
    std::string currentFolderName = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10";
    //QDir::setCurrent(currentFolderName);

    //std::string filename = "";
    //std::string filepattern = "BRUGGE_0010.S[0-9][0-9][0-9][0-9]";
    std::string filepattern = "BRUGGE_0010.S*";
    //std::string filepattern = "*";

    stringlist_type* names = stringlist_alloc_new();

    stringlist_select_matching_files(names, currentFolderName.data(), filepattern.data());

    for (int i = 0; i < stringlist_get_size(names); i++)
    {
        std::cout << stringlist_iget(names, i) << std::endl;
    }

    stringlist_free(names);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, DISABLED_StringlistSelectMatchingFilesQuestion)
{
    std::string currentFolderName = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10";
    std::string filepattern = "BRUGGE_0010.S????";

    stringlist_type* names = stringlist_alloc_new();

    stringlist_select_matching_files(names, currentFolderName.data(), filepattern.data());

    for (int i = 0; i < stringlist_get_size(names); i++)
    {
        std::cout << stringlist_iget(names, i) << std::endl;
    }

    stringlist_free(names);
}



/ *
WBHP:I01-01
WBHP:I01-02
WBHP:I01-03
WBHP:I02
WBHP:I02-01
WBHP:I02-02
WBHP:I02-03
WBHP:I03
WBHP:I03-01
WBHP:I03-02
WBHP:I03-03
WBHP:I04
WBHP:I04-01
WBHP:I04-02
WBHP:I04-03
WBHP:I05
WBHP:I05-01
WBHP:I05-02
WBHP:I05-03
WBHP:I06
WBHP:I06-01
WBHP:I06-02
WBHP:I06-03
WBHP:I07
WBHP:I07-01
WBHP:I07-02
WBHP:I07-03
WBHP:I08
WBHP:I08-01
WBHP:I08-02
WBHP:I08-03
WBHP:I09
WBHP:I09-01
WBHP:I09-02
WBHP:I09-03
WBHP:I10
WBHP:I10-01
WBHP:I10-02
WBHP:I10-03
WBHP:P01
WBHP:P01-01
WBHP:P01-02
WBHP:P01-03
WBHP:P02
WBHP:P02-01
WBHP:P02-02
WBHP:P02-03
WBHP:P03
WBHP:P03-01
WBHP:P03-02
WBHP:P03-03
WBHP:P04
WBHP:P04-01
WBHP:P04-02
WBHP:P04-03
WBHP:P05
WBHP:P05-01
WBHP:P05-02
WBHP:P06
WBHP:P06-01
WBHP:P06-02
WBHP:P07
WBHP:P07-01
WBHP:P07-02
WBHP:P07-03
WBHP:P08
WBHP:P08-01
WBHP:P08-02
WBHP:P08-03
WBHP:P09
WBHP:P09-01
WBHP:P10
WBHP:P10-01
WBHP:P10-02
WBHP:P11
WBHP:P11-01
WBHP:P11-02
WBHP:P11-03
WBHP:P12
WBHP:P12-01
WBHP:P12-02
WBHP:P12-03
WBHP:P13
WBHP:P13-01
WBHP:P13-02
WBHP:P13-03
WBHP:P14
WBHP:P14-01
WBHP:P14-02
WBHP:P15
WBHP:P15-01
WBHP:P15-02
WBHP:P16
WBHP:P16-01
WBHP:P16-02
WBHP:P16-03
WBHP:P17
WBHP:P17-01
WBHP:P17-02
WBHP:P17-03
WBHP:P18
WBHP:P18-01
WBHP:P18-02
WBHP:P18-03
WBHP:P19
WBHP:P19-01
WBHP:P19-02
WBHP:P19-03
WBHP:P20
WBHP:P20-01
WBHP:P20-02
WBHP:P20-03
WOPR:P01
WOPR:P01-01
WOPR:P01-02
WOPR:P01-03
WOPR:P02
WOPR:P02-01
WOPR:P02-02
WOPR:P02-03
WOPR:P03
WOPR:P03-01
WOPR:P03-02
WOPR:P03-03
WOPR:P04
WOPR:P04-01
WOPR:P04-02
WOPR:P04-03
WOPR:P05
WOPR:P05-01
WOPR:P05-02
WOPR:P06
WOPR:P06-01
WOPR:P06-02
WOPR:P07
WOPR:P07-01
WOPR:P07-02
WOPR:P07-03
WOPR:P08
WOPR:P08-01
WOPR:P08-02
WOPR:P08-03
WOPR:P09
WOPR:P09-01
WOPR:P10
WOPR:P10-01
WOPR:P10-02
WOPR:P11
WOPR:P11-01
WOPR:P11-02
WOPR:P11-03
WOPR:P12
WOPR:P12-01
WOPR:P12-02
WOPR:P12-03
WOPR:P13
WOPR:P13-01
WOPR:P13-02
WOPR:P13-03
WOPR:P14
WOPR:P14-01
WOPR:P14-02
WOPR:P15
WOPR:P15-01
WOPR:P15-02
WOPR:P16
WOPR:P16-01
WOPR:P16-02
WOPR:P16-03
WOPR:P17
WOPR:P17-01
WOPR:P17-02
WOPR:P17-03
WOPR:P18
WOPR:P18-01
WOPR:P18-02
WOPR:P18-03
WOPR:P19
WOPR:P19-01
WOPR:P19-02
WOPR:P19-03
WOPR:P20
WOPR:P20-01
WOPR:P20-02
WOPR:P20-03
WWIR:I01
WWIR:I01-01
WWIR:I01-02
WWIR:I01-03
WWIR:I02
WWIR:I02-01
WWIR:I02-02
WWIR:I02-03
WWIR:I03
WWIR:I03-01
WWIR:I03-02
WWIR:I03-03
WWIR:I04
WWIR:I04-01
WWIR:I04-02
WWIR:I04-03
WWIR:I05
WWIR:I05-01
WWIR:I05-02
WWIR:I05-03
WWIR:I06
WWIR:I06-01
WWIR:I06-02
WWIR:I06-03
WWIR:I07
WWIR:I07-01
WWIR:I07-02
WWIR:I07-03
WWIR:I08
WWIR:I08-01
WWIR:I08-02
WWIR:I08-03
WWIR:I09
WWIR:I09-01
WWIR:I09-02
WWIR:I09-03
WWIR:I10
WWIR:I10-01
WWIR:I10-02
WWIR:I10-03
WWPR:P01
WWPR:P01-01
WWPR:P01-02
WWPR:P01-03
WWPR:P02
WWPR:P02-01
WWPR:P02-02
WWPR:P02-03
WWPR:P03
WWPR:P03-01
WWPR:P03-02
WWPR:P03-03
WWPR:P04
WWPR:P04-01
WWPR:P04-02
WWPR:P04-03
WWPR:P05
WWPR:P05-01
WWPR:P05-02
WWPR:P06
WWPR:P06-01
WWPR:P06-02
WWPR:P07
WWPR:P07-01
WWPR:P07-02
WWPR:P07-03
WWPR:P08
WWPR:P08-01
WWPR:P08-02
WWPR:P08-03
WWPR:P09
WWPR:P09-01
WWPR:P10
WWPR:P10-01
WWPR:P10-02
WWPR:P11
WWPR:P11-01
WWPR:P11-02
WWPR:P11-03
WWPR:P12
WWPR:P12-01
WWPR:P12-02
WWPR:P12-03
WWPR:P13
WWPR:P13-01
WWPR:P13-02
WWPR:P13-03
WWPR:P14
WWPR:P14-01
WWPR:P14-02
WWPR:P15
WWPR:P15-01
WWPR:P15-02
WWPR:P16
WWPR:P16-01
WWPR:P16-02
WWPR:P16-03
WWPR:P17
WWPR:P17-01
WWPR:P17-02
WWPR:P17-03
WWPR:P18
WWPR:P18-01
WWPR:P18-02
WWPR:P18-03
WWPR:P19
WWPR:P19-01
WWPR:P19-02
WWPR:P19-03
WWPR:P20
WWPR:P20-01
WWPR:P20-02
WWPR:P20-03
YEARS
* /

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, DISABLED_BasicTestSetCurrentFolder)
{
/ *
    std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary);








    QString currentFolderName = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10";
    QDir::setCurrent(currentFolderName);

    std::string filename = "BRUGGE_0010";
    eclSummary->open(filename);

    std::vector<std::string> keywords;
    eclSummary->keywords(&keywords);

    for (size_t i = 0; i < keywords.size(); i++)
    {
        std::cout << keywords[i] << std::endl;
    }
* /
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseSummaryTest, DISABLED_BasicTest)
{
/ *
    std::unique_ptr<RifReaderEclipseSummary> eclSummary = std::unique_ptr<RifReaderEclipseSummary>(new
RifReaderEclipseSummary);








    std::string filename = "g:\\Models\\Statoil\\MultipleRealisations\\Case_without_p9\\Real10\\BRUGGE_0010";
    eclSummary->open(filename);








    std::vector<std::string> keywords;
    eclSummary->keywords(&keywords);

    for (size_t i = 0; i < keywords.size(); i++)
    {
        std::cout << keywords[i] << std::endl;
    }
* /
}
*/
