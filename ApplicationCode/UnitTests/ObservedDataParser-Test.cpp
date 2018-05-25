#include "gtest/gtest.h"

#include "RifColumnBasedUserData.h"
#include "RifColumnBasedUserDataParser.h"
#include "RifKeywordVectorParser.h"
#include "RifEclipseUserDataParserTools.h"
#include "RifCsvUserDataParser.h"
#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include <vector>
#include <QTextStream>
#include <QDebug>
#include "RifEclipseUserDataKeywordTools.h"
#include "RiaQDateTimeTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatYyyymmddWithDash)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "yyyy-MM-dd";
    parseOptions.cellSeparator = "\t";
    parseOptions.locale = QLocale::Norwegian;
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);
    out << "Date"       << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "1993-02-23" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "1993-06-15" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "1994-02-26" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "1994-05-23" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);
    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.dateTimeColumn() != nullptr);

    std::vector<QDateTime> timeSteps = parser.dateTimeColumn()->dateTimeValues;

    ASSERT_EQ(size_t(4), timeSteps.size());
    EXPECT_EQ("1993-02-23", timeSteps[0].toString(parseOptions.dateFormat).toStdString());
    EXPECT_EQ("1993-06-15", timeSteps[1].toString(parseOptions.dateFormat).toStdString());
    EXPECT_EQ("1994-02-26", timeSteps[2].toString(parseOptions.dateFormat).toStdString());
    EXPECT_EQ("1994-05-23", timeSteps[3].toString(parseOptions.dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatYymmddWithDot)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "yy.MM.dd";
    parseOptions.cellSeparator = "\t";
    parseOptions.locale = QLocale::Norwegian;
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "93.02.23" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "93.06.15" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "94.02.26" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "94.05.23" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);

    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.dateTimeColumn() != nullptr);

    std::vector<QDateTime> timeSteps = parser.dateTimeColumn()->dateTimeValues;

    ASSERT_EQ(size_t(4), timeSteps.size());
    EXPECT_EQ("93.02.23", timeSteps[0].toString(parseOptions.dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatDdmmyyWithDot)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "dd.MM.yy";
    parseOptions.cellSeparator = "\t";
    parseOptions.locale = QLocale::Norwegian;
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "23.02.93" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "15.06.93" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "26.02.94" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "23.05.94" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);
    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.dateTimeColumn() != nullptr);

    std::vector<QDateTime> timeSteps = parser.dateTimeColumn()->dateTimeValues;

    ASSERT_EQ(size_t(4), timeSteps.size());
    EXPECT_EQ("23.02.93", timeSteps[0].toString(parseOptions.dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDecimalLocaleNorwegian)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "yy.MM.dd";
    parseOptions.cellSeparator = "\t";
    parseOptions.decimalSeparator = ",";
    parseOptions.locale = QLocale::Norwegian;
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);
    out << "Date" << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "93.02.23" << "\t" << "10,1" << "\t" << "1,0" << "\n";
    out << "93.06.15" << "\t" << "20,40" << "\t" << "2,33" << "\n";
    out << "94.02.26" << "\t" << "30,2" << "\t" << "3,09" << "\n";
    out << "94.05.23" << "\t" << "40,8" << "\t" << "4,44" << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);

    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.columnInfo(1) != nullptr);
    ASSERT_TRUE(parser.columnInfo(2) != nullptr);

    std::vector<double> oilValues = parser.columnInfo(1)->values;
    std::vector<double> pwValues = parser.columnInfo(2)->values;

    ASSERT_EQ(size_t(4), oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(size_t(4), pwValues.size());
    EXPECT_EQ(1.0, pwValues[0]);
    EXPECT_EQ(1, pwValues[0]);
    EXPECT_EQ(2.33, pwValues[1]);
    EXPECT_EQ(3.09, pwValues[2]);
    EXPECT_EQ(4.44, pwValues[3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDecimalLocaleC)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "yy.MM.dd";
    parseOptions.cellSeparator = "\t";
    parseOptions.locale = QLocale::c();
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil"   << "\t" << "PW"   << "\t" << "H2S"  << "\n";
    out << "93.02.23" << "\t" << "10.1"  << "\t" << "1.0"  << "\t" << "0.2"  << "\n";
    out << "93.06.15" << "\t" << "20.40" << "\t" << "2.33" << "\t" << "2.13" << "\n";
    out << "94.02.26" << "\t" << "30.2"  << "\t" << "3.09" << "\t" << "2.1"  << "\n";
    out << "94.05.23" << "\t" << "40.8"  << "\t" << "4.44" << "\t" << "1.0"  << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);

    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.columnInfo(1) != nullptr);
    ASSERT_TRUE(parser.columnInfo(2) != nullptr);
    ASSERT_TRUE(parser.columnInfo(3) != nullptr);

    std::vector<double> oilValues = parser.columnInfo(1)->values;
    std::vector<double> pwValues = parser.columnInfo(2)->values;
    std::vector<double> h2sValues = parser.columnInfo(3)->values;

    ASSERT_EQ(size_t(4), oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(size_t(4), pwValues.size());
    EXPECT_EQ(1.0, pwValues[0]);
    EXPECT_EQ(2.33, pwValues[1]);
    EXPECT_EQ(3.09, pwValues[2]);
    EXPECT_EQ(4.44, pwValues[3]);

    ASSERT_EQ(size_t(4), h2sValues.size());
    EXPECT_EQ(0.2, h2sValues[0]);
    EXPECT_EQ(2.13, h2sValues[1]);
    EXPECT_EQ(2.1, h2sValues[2]);
    EXPECT_EQ(1.0, h2sValues[3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestCellSeparatorComma)
{
    AsciiDataParseOptions parseOptions;
    parseOptions.dateFormat = "yy.MM.dd";
    parseOptions.cellSeparator = ",";
    parseOptions.locale = QLocale::c();
    parseOptions.timeSeriesColumnName = "Date";

    QString data;
    QTextStream out(&data);

    out << "Date"     << "," << "Oil"   << "," << "PW"   << "\n";
    out << "93.02.23" << "," << "10.1"  << "," << "1.0"  << "\n";
    out << "93.06.15" << "," << "20.40" << "," << "2.33" << "\n";
    out << "94.02.26" << "," << "30.2"  << "," << "3.09" << "\n";
    out << "94.05.23" << "," << "40.8"  << "," << "4.44" << "\n";

    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);

    ASSERT_TRUE(parser.parse(parseOptions));
    ASSERT_TRUE(parser.columnInfo(1) != nullptr);
    ASSERT_TRUE(parser.columnInfo(2) != nullptr);

    std::vector<double> oilValues = parser.columnInfo(1)->values;
    std::vector<double> pwValues = parser.columnInfo(2)->values;

    ASSERT_EQ(size_t(4), oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(size_t(4), pwValues.size());
    EXPECT_EQ(1.0, pwValues[0]);
    EXPECT_EQ(2.33, pwValues[1]);
    EXPECT_EQ(3.09, pwValues[2]);
    EXPECT_EQ(4.44, pwValues[3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifRsmspecParserToolsTest, TestSplitLineToDoubles)
{

    QString data;
    QTextStream out(&data);

    out << "   1   0.0   0.0   0.0   0.0   0.0\n";
    out << "   2   0.0   0.0   0.0   0.0   0.0\n";
    out << "   3   0.0   0.0   0.0   0.0   0.0 \n";
    out << "   4   0.0   0.0   0.0   0.0   0.0  --note\n";
    out << "   5   0.0   0.0   0.0   0.0   0.0\n";
    out << "   6   0.0   0.0   0.0   0.0   0.0\n";
    out << "   7  0.0  0.0  282  0.0  0.0 -- This is a test \n";
    out << "   8  0.0  0.0  279  0.0  0.0\n";
    out << "   9   0.0   0.0   0.0   0.0   0.0\n";
    out << "   10   0.0   0.0   0.0   0.0   0.0\n";

    std::stringstream streamData;
    streamData.str(data.toStdString());
    std::string line;

    std::vector< std::vector<double> > table;

    while (std::getline(streamData, line))
    {
        std::vector<double> values = RifEclipseUserDataParserTools::splitLineToDoubles(line);
        table.push_back(values);
    }

    ASSERT_EQ(size_t(10), table.size());
    ASSERT_EQ(size_t(6), table[0].size());
    ASSERT_EQ(size_t(6), table[3].size());

    EXPECT_EQ(size_t(1), table[0][0]);
    EXPECT_EQ(0.0, table[5][2]);
    EXPECT_EQ(size_t(279), table[7][3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, TestTwoPages)
{

    QString data;
    QTextStream out(&data);

    out << "PAGE 1\n";
    out << "ORIGIN    OP-1_TR\n";
    out << "STARTDATE    1 1 2000\n";
    out << "DATEFORMAT        DD/MM/YY\n";
    out << "\n";
    out << "TIME    YEARX    WGT1    WGT2    WGT4    WR12    WR22    WR42    \n";
    out << "DAYS    YEARS    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3  kg/Sm3        \n";
    out << "1    1    1.00E+03    1.00E+03    1.00E+03    1.00E+03    1.00E+03    1.00E+03    \n";
    out << "        OP-1    OP-1    OP-1    OP-1    OP-1    OP-1    \n";
    out << "\n";
    out << "\n";
    out << "1386    2003.170    1.0E-12       1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12  --comment \n";
    out << "1436    2003.307    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12 --comment\n";
    out << "1574    2003.685    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1636    2003.855    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1709    2004.055    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "\n";
    out << "\n";
    out << "PAGE 2\n";
    out << "ORIGIN    OP-2_TR    \n";
    out << "STARTDATE    1 1 2000\n";
    out << "DATEFORMAT        DD/MM/YY\n";
    out << "\n";
    out << "TIME    YEARX    WGT1    WGT2    WGT4    WR12    WR22    WR42    WR31\n";
    out << "DAYS    YEARS    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3    kg/Sm3\n";
    out << "1    1    1.00E+03    1.00E+03    1.00E+03    1.00E+03    1.00E+03    1.00E+03    1.00E+00\n";
    out << "        OP-2    OP-2    OP-2    OP-2    OP-2    OP-2    OP-2\n";
    out << "\n";
    out << "1436    2003.307    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1508    2003.504    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1574    2003.685    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1636    2003.855    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "1709    2004.055    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12    1.0E-12\n";
    out << "\n";

    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();
    ASSERT_EQ(size_t(2), tables.size());
    EXPECT_EQ("1 1 2000", tables[0].startDate());
    EXPECT_EQ("OP-1_TR", tables[0].origin());

    EXPECT_EQ("1 1 2000", tables[1].startDate());
    EXPECT_EQ("OP-2_TR", tables[1].origin());

    ASSERT_EQ(size_t(8), tables.at(0).columnInfos().size());
    EXPECT_EQ(1.0E-12, tables.at(0).columnInfos().at(4).values[0]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, TestTableValues)
{

    QString data;
    QTextStream out(&data);

    out << "1\n";
    out << "---------------------------------------\n";
    out << "SUMMARY OF RUN SIP USER FILE DATA VECTORS\n";
    out << "---------------------------------------\n";
    out << "TIME     WLVP       WPLT      WTEST      WFOFF      WBUP\n";
    out << "DAYS      BARSA       BARSA     BARSA     BARSA    BARSA\n";
    out << "\n";
    out << "    P-15P   P-15P   P-15P   P-15P   P-15P  \n";
    out << "   1   0.0   0.0   0.0   0.0   0.0\n";
    out << "   2   0.0   0.0   0.0   0.0   0.0\n";
    out << "   3   0.0   0.0   0.0   0.0   0.0\n";
    out << "   4   0.0   0.0   0.0   0.0   0.0\n";
    out << "   5   0.0   0.0   0.0   0.0   0.0\n";
    out << "   6   0.0   0.0   0.0   0.0   0.0\n";
    out << "   7  0.0  0.0  282  0.0  0.0\n";
    out << "   8  0.0  0.0  279  0.0  0.0\n";
    out << "   9   0.0   0.0   0.0   0.0   0.0\n";
    out << "   10   0.0   0.0   0.0   0.0   0.0\n";
    out << "   11   0.0   0.0   0.0   0.0   0.0\n";
    out << "   12   0.0   0.0   0.0   0.0   0.0\n";
    out << "   13   0.0   0.0   0.0   0.0   0.0\n";
    out << "   14   0.0   0.0   0.0   0.0   0.0\n";
    out << "   15   0.0   0.0   0.0   0.0   0.0\n";
    out << "   16   0.0   0.0   0.0   0.0   0.0\n";
    out << "   17   0.0   0.0   0.0   0.0   0.0\n";
    out << "   18   0.0   0.0   0.0   0.0   0.0\n";
    out << "1\n";
    out << "---------------------------------------\n";
    out << "SUMMARY OF RUN SIP USER FILE DATA VECTORS\n";
    out << "---------------------------------------\n";
    out << "TIME     WLVP       WPLT      WTEST      WFOFF      WBUP\n";
    out << "DAYS      BARSA       BARSA     BARSA     BARSA    BARSA\n";
    out << "\n";
    out << "    P-9P   P-9P   P-9P   P-9P   P-9P  \n";
    out << "   1   0.0   0.0   0.0   0.0   0.0\n";
    out << "   2   0.0   0.0   0.0   0.0   0.0\n";
    out << "   3   0.0   0.0   0.0   0.0   0.0\n";
    out << "   4  0.0  0.0  370  0.0  0.0\n";
    out << "\n";

    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();
    ASSERT_EQ(size_t(2), tables.size());
    
    ASSERT_EQ(size_t(6), tables.at(0).columnInfos().size());
    ASSERT_EQ(size_t(6), tables.at(1).columnInfos().size());

    ASSERT_EQ(size_t(18), tables.at(0).columnInfos().at(0).values.size());
    ASSERT_EQ(size_t(4), tables.at(1).columnInfos().at(0).values.size());

    EXPECT_EQ(0.0, tables.at(0).columnInfos().at(1).values.at(6));
    EXPECT_EQ(282.0, tables.at(0).columnInfos().at(3).values.at(6));

    EXPECT_EQ(3.0, tables.at(1).columnInfos().at(0).values.at(2));
    EXPECT_EQ(370.0, tables.at(1).columnInfos().at(3).values.at(3));

    EXPECT_EQ("WLVP", tables.at(0).columnInfos().at(1).summaryAddress.quantityName());
    EXPECT_EQ("P-15P", tables.at(0).columnInfos().at(5).summaryAddress.wellName());
    EXPECT_EQ("P-9P", tables.at(1).columnInfos().at(1).summaryAddress.wellName());
    EXPECT_NE("P-9P", tables.at(1).columnInfos().at(0).summaryAddress.wellName());

    RifColumnBasedUserData userData;
    userData.parse(data);

    RifEclipseSummaryAddress adr(RifEclipseSummaryAddress::SUMMARY_WELL, "WLVP", -1, -1, "", "P-15P", -1, "", -1, -1, -1, -1, false);
   
    QDateTime firstTimeStep = RiaQDateTimeTools::addDays(RiaQDateTimeTools::epoch(), 1.0);
    auto timeSteps = userData.timeSteps(adr);
    EXPECT_EQ(size_t(18), timeSteps.size());

    EXPECT_EQ(firstTimeStep.toTime_t(), timeSteps[0]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, TestTableMissingWellNames)
{

    QString data;
    QTextStream out(&data);

    out << "1\n";
    out << "---------------------------------------\n";
    out << "SUMMARY OF RUN SIP USER FILE DATA VECTORS\n";
    out << "---------------------------------------\n";
    out << "TIME     WLVP       WPLT      WTEST      WFOFF      WBUP\n";
    out << "DAYS      BARSA       BARSA     BARSA     BARSA    BARSA\n";
    out << "\n";
    out << "   1   0.0   0.0   0.0   0.0   0.0\n";
    out << "   2   0.0   0.0   0.0   0.0   0.0\n";
    out << "   3   0.0   0.0   0.0   0.0   0.0\n";
    out << "   4   0.0   0.0   0.0   0.0   0.0\n";
    out << "   5   0.0   0.0   0.0   0.0   0.0\n";
    out << "   6   0.0   0.0   0.0   0.0   0.0\n";
    out << "   7  0.0  0.0  282  0.0  0.0\n";
    out << "   8  0.0  0.0  279  0.0  0.0\n";
    out << "   9   0.0   0.0   0.0   0.0   0.0\n";
    out << "   10   0.0   0.0   0.0   0.0   0.0\n";
    out << "   11   0.0   0.0   0.0   0.0   0.0\n";
    out << "   12   0.0   0.0   0.0   0.0   0.0\n";
    out << "   13   0.0   0.0   0.0   0.0   0.0\n";
    out << "   14   0.0   0.0   0.0   0.0   0.0\n";
    out << "   15   0.0   0.0   0.0   0.0   0.0\n";
    out << "   16   0.0   0.0   0.0   0.0   0.0\n";
    out << "   17   0.0   0.0   0.0   0.0   0.0\n";
    out << "   18   0.0   0.0   0.0   0.0   0.0\n";

    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();

    // Missing header line with well name, returning empty table
    ASSERT_EQ(size_t(0), tables.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, TestTableValuesHeaderWithSpaces)
{
    QString data;
    QTextStream out(&data);

    out << "1\n";
    out << " -----------------------------------------\n";
    out << " SUMMARY OF RUN NORNE_ATW2013_RFTPLT_V3 EC\n";
    out << " -----------------------------------------\n";
    out << " DATE         YEARS        WBHP           \n";
    out << " DATE         YEARS        BARSA          \n";
    out << "                           B-1H           \n";
    out << "                                          \n";
    out << " -----------------------------------------\n";
    out << "  6-NOV-1997         0           0        \n";
    out << "  7-NOV-1997  0.002738           0        \n";
    out << "  8-NOV-1997  0.006556           0        \n";
    out << "  9-NOV-1997  0.009231           0        \n";
    out << " 10-NOV-1997  0.011462           0        \n";
    out << " 11-NOV-1997  0.013710           0        \n";
    out << " 11-NOV-1997  0.015950           0        \n";
    out << " 12-NOV-1997  0.018477           0        \n";
    out << " 13-NOV-1997  0.020190           0        \n";
    out << " 14-NOV-1997  0.021903           0        \n";
    out << " 14-NOV-1997  0.024232           0        \n";
    out << " 17-NOV-1997  0.030838           0        \n";
    out << " 19-NOV-1997  0.037060           0        \n";
    out << " 21-NOV-1997  0.042841           0        \n";
    out << " 23-NOV-1997  0.049239           0        \n";
    out << " 25-NOV-1997  0.054613           0        \n";
    out << " 28-NOV-1997  0.061135           0        \n";
    out << " 29-NOV-1997  0.064791           0        \n";
    out << "  1-DEC-1997  0.068446           0        \n";
    out << "  2-DEC-1997  0.073016           0        \n";
    out << "  4-DEC-1997  0.078727           0        \n";
    out << "  7-DEC-1997  0.085750           0        \n";
    out << " 10-DEC-1997  0.093124           0        \n";
    out << " 13-DEC-1997  0.101750           0        \n";
    out << " 15-DEC-1997  0.107001           0        \n";
    out << " 17-DEC-1997  0.112252           0        \n";
    
    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();
    ASSERT_EQ(size_t(1), tables.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, TestTableDateOnly)
{
    QString data;
    QTextStream out(&data);

    out << "1\n";
    out << " -----------------------------------------\n";
    out << " SUMMARY OF RUN NORNE_ATW2013_RFTPLT_V3 EC\n";
    out << " -----------------------------------------\n";
    out << " DATE        WBHP           \n";
    out << " DATE        BARSA          \n";
    out << "             B-1H           \n";
    out << "                            \n";
    out << " ---------------------------\n";
    out << "  6-NOV-1997       0        \n";
    out << "  7-NOV-1997       0        \n";
    out << "  8-NOV-1997       0        \n";
    out << "  9-NOV-1997       0        \n";
    out << " 10-NOV-1997       0        \n";
    out << " 11-NOV-1997       0        \n";
    out << " 11-NOV-1997       0        \n";
    out << " 12-NOV-1997       0        \n";
    out << " 13-NOV-1997       0        \n";
    out << " 14-NOV-1997       0        \n";
    out << " 14-NOV-1997       0        \n";
    out << " 17-NOV-1997       0        \n";
    out << " 19-NOV-1997       0        \n";
    out << " 21-NOV-1997       0        \n";
    out << " 23-NOV-1997       0        \n";
    out << " 25-NOV-1997       0        \n";
    out << " 28-NOV-1997       0        \n";
    out << " 29-NOV-1997       0        \n";
    out << "  1-DEC-1997       0        \n";
    out << "  2-DEC-1997       0        \n";
    out << "  4-DEC-1997       0        \n";
    out << "  7-DEC-1997       0        \n";
    out << " 10-DEC-1997       0        \n";
    out << " 13-DEC-1997       0        \n";
    out << " 15-DEC-1997       0        \n";
    out << " 17-DEC-1997       0        \n";

    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();
    ASSERT_EQ(size_t(1), tables.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifKeywordBasedRsmspecParserTest, TestKeywordBasedVectorsValues)
{
    QString data;
    QTextStream out(&data);

    out << "----------------------------------------------\n";
    out << "-- GOR data S-1AH\n";
    out << "-- Based on well tests\n";
    out << "----------------------------------------------\n";
    out << "VECTOR S-1AH-GOR\n";
    out << "UNITS SM3/SM3\n";
    out << "ORIGIN GORS-1AH\n";
    out << "330.6601\n";
    out << "257.7500\n";
    out << "335.9894\n";
    out << "301.4388\n";
    out << "260.4193\n";
    out << "306.0298\n";
    out << "280.2883\n";
    out << "\n";
    out << "VECTOR YEARX\n";
    out << "ORIGIN GORS-1AH\n";
    out << "UNITS YEAR\n";
    out << "1999.7902\n";
    out << "1999.8446\n";
    out << "1999.9285\n";
    out << "2000.0391\n";
    out << "2000.0800\n";
    out << "2000.0862\n";
    out << "2000.1285\n";
    out << "---sjekk dato\n";
    out << "\n";
    out << "\n";
    out << "\n";
    out << "----------------------------------------------\n";
    out << "-- GOR data S-2H\n";
    out << "-- Based on well tests\n";
    out << "----------------------------------------------\n";
    out << "VECTOR S-2H-GOR\n";
    out << "UNITS SM3/SM3\n";
    out << "ORIGIN GORS-2H\n";
    out << "293.8103\n";
    out << "293.1634\n";
    out << "304.0000\n";
    out << "334.5932\n";
    out << "306.4610\n";
    out << "293.7571\n";
    out << "305.2013\n";
    out << "\n";
    out << "VECTOR YEARX\n";
    out << "ORIGIN GORS-2H\n";
    out << "UNITS YEAR\n";
    out << "1999.8255\n";
    out << "2000.1274\n";
    out << "2000.2075\n";
    out << "2000.2367\n";
    out << "2000.4033\n";
    out << "2000.4966\n";
    out << "2000.6832\n";
    out << "\n";
    out << "\n";

    RifKeywordVectorParser parser(data);

    std::vector<KeywordBasedVector> vectors = parser.keywordBasedVectors();

    EXPECT_TRUE(RifKeywordVectorParser::canBeParsed(data));

    ASSERT_EQ(size_t(4), vectors.size());

    ASSERT_LE(size_t(1), vectors[0].header.size());
    EXPECT_EQ("VECTOR S-1AH-GOR", vectors[0].header.at(0));

    ASSERT_LE(size_t(3), vectors[0].values.size());
    EXPECT_EQ(335.9894, vectors[0].values[2]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifKeywordBasedRsmspecParserTest, TestCannotBeParsed)
{
    QString data;
    QTextStream out(&data);

    out << "----------------------------------------------\n";
    out << "-- GOR data S-1AH\n";
    out << "----------------------------------------------\n";
    out << "UNITS SM3/SM3\n";
    out << "ORIGIN GORS-1AH\n";
    out << "330.6601\n";
    out << "335.9894\n";

    RifKeywordVectorParser parser(data);

    std::vector<KeywordBasedVector> vectors = parser.keywordBasedVectors();

    EXPECT_FALSE(RifKeywordVectorParser::canBeParsed(data));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifKeywordBasedRsmspecParserTest, TestShutins)
{
    QString data;
    QTextStream out(&data);

    out << "-- Created running the command shutin_pressures\n";
    out << "\n";
    out << "PAGE 1\n";
    out << "ORIGIN       OP-1_WBP9L\n";
    out << "STARTDATE    01 01 2004     -- DD MM YYYY\n";
    out << "\n";
    out << "TIME     YEARX      WBP9L\n";
    out << "DAYS     YEARS      BARSA\n";
    out << "1        1          1\n";
    out << "                    OP-1\n";
    out << "\n";
    out << "\n";
    out << "3043     2014.32    52.5     -- Extrapolated\n";
    out << "3046     2014.32    208.8    -- Measured\n";
    out << "3070     2014.39    197.6    -- Measured\n";
    out << "3081     2014.42    200.3    -- Measured\n";
    out << "3128     2014.55    203.3    -- Measured\n";
    out << "3141     2014.59    198.0    -- Measured\n";
    out << "3196     2014.73    197.2    -- Measured\n";
    out << "3222     2014.81    196.9    -- Extrapolated\n";
    out << "3225     2014.82    199.6    -- Extrapolated\n";
    out << "3226     2014.82    200.0    -- Extrapolated\n";
    out << "3247     2014.87    201.8    -- Extrapolated\n";
    out << "3282     2014.97    51.7     -- Extrapolated\n";
    out << "3282     2014.97    201.6    -- Measured\n";
    out << "3304     2015.03    206.1    -- Extrapolated\n";
    out << "3324     2015.09    170.2    -- Measured\n";
    out << "3359     2015.18    207.0    -- Extrapolated\n";
    out << "3481     2015.52    151.0    -- Measured\n";
    out << "3493     2015.55    219.0    -- Measured\n";
    out << "\n";



    RifColumnBasedUserDataParser parser = RifColumnBasedUserDataParser(data);

    auto tables = parser.tableData();
    ASSERT_EQ(size_t(1), tables.size());
    ASSERT_EQ(size_t(3), tables[0].columnInfos().size());
    ASSERT_EQ(size_t(18), tables.at(0).columnInfos().at(2).values.size());

    EXPECT_EQ(2014.39, tables.at(0).columnInfos().at(1).values[2]);

    EXPECT_EQ("WBP9L", tables.at(0).columnInfos().at(2).summaryAddress.quantityName());

    EXPECT_EQ("OP-1", tables.at(0).columnInfos().at(2).summaryAddress.wellName());
    EXPECT_NE("OP-1", tables.at(0).columnInfos().at(1).summaryAddress.wellName());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifKeywordBasedRsmspecParserTest, TestTimeSteps)
{
    QString data;
    QTextStream out(&data);

    out << "-- Created running the command shutin_pressures\n";
    out << "\n";
    out << "PAGE 1\n";
    out << "ORIGIN       OP-1_WBP9L\n";
    out << "STARTDATE    01 01 2004     -- DD MM YYYY\n";
    out << "\n";
    out << "TIME     YEARX      WBP9L\n";
    out << "DAYS     YEARS      BARSA\n";
    out << "1        1          1\n";
    out << "                    OP-1\n";
    out << "\n";
    out << "\n";
    out << "3043     2014.32    52.5     -- Extrapolated\n";
    out << "3046     2014.32    208.8    -- Measured\n";
    out << "3070     2014.39    197.6    -- Measured\n";
    out << "3081     2014.42    200.3    -- Measured\n";
    out << "3128     2014.55    203.3    -- Measured\n";
    out << "3141     2014.59    198.0    -- Measured\n";
    out << "3196     2014.73    197.2    -- Measured\n";
    out << "3222     2014.81    196.9    -- Extrapolated\n";
    out << "3225     2014.82    199.6    -- Extrapolated\n";
    out << "3226     2014.82    200.0    -- Extrapolated\n";
    out << "3247     2014.87    201.8    -- Extrapolated\n";
    out << "3282     2014.97    51.7     -- Extrapolated\n";
    out << "3282     2014.97    201.6    -- Measured\n";
    out << "3304     2015.03    206.1    -- Extrapolated\n";
    out << "3324     2015.09    170.2    -- Measured\n";
    out << "3359     2015.18    207.0    -- Extrapolated\n";
    out << "3481     2015.52    151.0    -- Measured\n";
    out << "3493     2015.55    219.0    -- Measured\n";
    out << "\n";

    std::string quantityName = "WBP9L";
    std::vector< std::string > headerColumn;
    headerColumn.push_back("OP-1");

    RifEclipseSummaryAddress address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantityName, headerColumn);

    RifColumnBasedUserData columnBasedUserdata;

    columnBasedUserdata.parse(data);
    std::vector<time_t> timeSteps = columnBasedUserdata.timeSteps(address);

    QDateTime firstDate = RiaQDateTimeTools::fromYears(2014.32);

    EXPECT_TRUE(firstDate == QDateTime::fromTime_t(timeSteps[0]));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifKeywordBasedRsmspecParserTest, TestAddressCreation)
{
    std::string quantityName = "LCABC";
    std::vector< std::string > headerColumn;
    headerColumn.push_back("wellName");
    headerColumn.push_back("lgrName");
    headerColumn.push_back("12 14 16");

    RifEclipseSummaryAddress address = RifEclipseUserDataKeywordTools::makeAndFillAddress(quantityName, headerColumn);

    EXPECT_TRUE(address.isValid());
    EXPECT_EQ(address.category(), RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR);
    EXPECT_EQ(address.uiText(), "LCABC:lgrName:wellName:12, 14, 16");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, IsTableData)
{
    {
        std::string line("  6-NOV-1997       0        ");
        EXPECT_TRUE(RifEclipseUserDataParserTools::isValidTableData(2, line));
    }

    {
        std::string line("  DATE BARSA        ");
        EXPECT_FALSE(RifEclipseUserDataParserTools::isValidTableData(2, line));
    }

    {
        std::string line("  1.2       0        ");
        EXPECT_TRUE(RifEclipseUserDataParserTools::isValidTableData(2, line));
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedRsmspecParserTest, HasOnlyValidDoubleValues)
{
    {
        std::vector<double> doubleValues;
        std::string line("  6-NOV-1997       0        ");
        std::vector<std::string> words = RifEclipseUserDataParserTools::splitLineAndRemoveComments(line);
        
        EXPECT_FALSE(RifEclipseUserDataParserTools::hasOnlyValidDoubleValues(words, &doubleValues));
    }
}
