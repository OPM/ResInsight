#include "gtest/gtest.h"

#include "RifColumnBasedAsciiParser.h"
#include "RifColumnBasedRsmspecParser.h"
#include "RifKeywordVectorParser.h"
#include "RifRsmspecParserTools.h"

#include <vector>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatYyyymmddWithDash)
{
    QString dateFormat = "yyyy-MM-dd";
    QString cellSeparator = "\t";
    QLocale decimalLocale = QLocale::Norwegian;

    QString data;
    QTextStream out(&data);
    out << "Date"       << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "1993-02-23" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "1993-06-15" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "1994-02-26" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "1994-05-23" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);
    
    std::vector<QDateTime> timeSteps = parser.timeSteps();

    ASSERT_EQ(4, parser.timeSteps().size());
    EXPECT_EQ("1993-02-23", timeSteps[0].toString(dateFormat).toStdString());
    EXPECT_EQ("1993-06-15", timeSteps[1].toString(dateFormat).toStdString());
    EXPECT_EQ("1994-02-26", timeSteps[2].toString(dateFormat).toStdString());
    EXPECT_EQ("1994-05-23", timeSteps[3].toString(dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatYymmddWithDot)
{
    QString dateFormat = "yy.MM.dd";
    QString cellSeparator = "\t";
    QLocale decimalLocale = QLocale::Norwegian;

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "93.02.23" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "93.06.15" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "94.02.26" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "94.05.23" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);

    std::vector<QDateTime> timeSteps = parser.timeSteps();

    ASSERT_EQ(4, parser.timeSteps().size());
    EXPECT_EQ("93.02.23", timeSteps[0].toString(dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDateFormatDdmmyyWithDot)
{
    QString dateFormat = "dd.MM.yy";
    QString cellSeparator = "\t";
    QLocale decimalLocale = QLocale::Norwegian;

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "23.02.93" << "\t" << "10"  << "\t" << "1"  << "\n";
    out << "15.06.93" << "\t" << "20"  << "\t" << "2"  << "\n";
    out << "26.02.94" << "\t" << "30"  << "\t" << "3"  << "\n";
    out << "23.05.94" << "\t" << "40"  << "\t" << "4"  << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);

    std::vector<QDateTime> timeSteps = parser.timeSteps();

    ASSERT_EQ(4, parser.timeSteps().size());
    EXPECT_EQ("23.02.93", timeSteps[0].toString(dateFormat).toStdString());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifColumnBasedAsciiParserTest, TestDecimalLocaleNorwegian)
{
    QString dateFormat = "yy.MM.dd";
    QString cellSeparator = "\t";
    QLocale decimalLocale = QLocale::Norwegian;

    QString data;
    QTextStream out(&data);
    out << "Date" << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "93.02.23" << "\t" << "10,1" << "\t" << "1,0" << "\n";
    out << "93.06.15" << "\t" << "20,40" << "\t" << "2,33" << "\n";
    out << "94.02.26" << "\t" << "30,2" << "\t" << "3,09" << "\n";
    out << "94.05.23" << "\t" << "40,8" << "\t" << "4,44" << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);

    std::vector<double> oilValues = parser.columnValues(0);
    std::vector<double> pwValues = parser.columnValues(1);

    ASSERT_EQ(4, oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(4, pwValues.size());
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
    QString dateFormat = "yy.MM.dd";
    QString cellSeparator = "\t";
    QLocale decimalLocale = QLocale::c();

    QString data;
    QTextStream out(&data);
    out << "Date"     << "\t" << "Oil"   << "\t" << "PW"   << "\t" << "H2S"  << "\n";
    out << "93.02.23" << "\t" << "10.1"  << "\t" << "1.0"  << "\t" << "0.2"  << "\n";
    out << "93.06.15" << "\t" << "20.40" << "\t" << "2.33" << "\t" << "2.13" << "\n";
    out << "94.02.26" << "\t" << "30.2"  << "\t" << "3.09" << "\t" << "2.1"  << "\n";
    out << "94.05.23" << "\t" << "40.8"  << "\t" << "4.44" << "\t" << "1.0"  << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);

    std::vector<double> oilValues = parser.columnValues(0);
    std::vector<double> pwValues = parser.columnValues(1);
    std::vector<double> h2sValues = parser.columnValues(2);

    ASSERT_EQ(4, oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(4, pwValues.size());
    EXPECT_EQ(1.0, pwValues[0]);
    EXPECT_EQ(2.33, pwValues[1]);
    EXPECT_EQ(3.09, pwValues[2]);
    EXPECT_EQ(4.44, pwValues[3]);

    ASSERT_EQ(4, h2sValues.size());
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
    QString dateFormat = "yy.MM.dd";
    QString cellSeparator = ",";
    QLocale decimalLocale = QLocale::c();

    QString data;
    QTextStream out(&data);

    out << "Date"     << "," << "Oil"   << "," << "PW"   << "\n";
    out << "93.02.23" << "," << "10.1"  << "," << "1.0"  << "\n";
    out << "93.06.15" << "," << "20.40" << "," << "2.33" << "\n";
    out << "94.02.26" << "," << "30.2"  << "," << "3.09" << "\n";
    out << "94.05.23" << "," << "40.8"  << "," << "4.44" << "\n";

    RifColumnBasedAsciiParser parser = RifColumnBasedAsciiParser(data, dateFormat, decimalLocale, cellSeparator);

    std::vector<double> oilValues = parser.columnValues(0);
    std::vector<double> pwValues = parser.columnValues(1);

    ASSERT_EQ(4, oilValues.size());
    EXPECT_EQ(10.1, oilValues[0]);
    EXPECT_EQ(20.40, oilValues[1]);
    EXPECT_EQ(30.2, oilValues[2]);
    EXPECT_EQ(40.8, oilValues[3]);

    ASSERT_EQ(4, pwValues.size());
    EXPECT_EQ(1.0, pwValues[0]);
    EXPECT_EQ(2.33, pwValues[1]);
    EXPECT_EQ(3.09, pwValues[2]);
    EXPECT_EQ(4.44, pwValues[3]);
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

    RifColumnBasedRsmspecParser parser = RifColumnBasedRsmspecParser(data);

    std::vector< std::vector<ColumnInfo> > tables = parser.tables();
    ASSERT_EQ(tables.size(), 2);
    
    ASSERT_EQ(tables.at(0).size(), 6);
    ASSERT_EQ(tables.at(1).size(), 6);

    ASSERT_EQ(18, tables.at(0).at(0).values.size());
    ASSERT_EQ(4, tables.at(1).at(0).values.size());

    EXPECT_EQ(0.0, tables.at(0).at(1).values.at(6));
    EXPECT_EQ(282, tables.at(0).at(3).values.at(6));

    EXPECT_EQ(3, tables.at(1).at(0).values.at(2));
    EXPECT_EQ(370, tables.at(1).at(3).values.at(3));

    EXPECT_EQ("WLVP", tables.at(0).at(1).summaryAddress.quantityName());
    EXPECT_EQ("P-9P", tables.at(1).at(1).summaryAddress.wellName());
    EXPECT_NE("P-9P", tables.at(1).at(0).summaryAddress.wellName());
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

    RifRsmspecParserTools parserTool;
    std::stringstream streamData;
    streamData.str(data.toStdString());
    std::string line;
    
    std::vector< std::vector<double> > table;
    
    while (std::getline(streamData, line))
    {
        std::vector<double> values;
        parserTool.splitLineToDoubles(line, values);
        table.push_back(values);
    }

    ASSERT_EQ(10, table.size());
    ASSERT_EQ(6, table[0].size());

    EXPECT_EQ(1, table[0][0]);
    EXPECT_EQ(0.0, table[5][2]);
    EXPECT_EQ(279, table[7][3]);
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

    ASSERT_EQ(4, vectors.size());

    ASSERT_LE(1, vectors[0].header.size());
    EXPECT_EQ("VECTOR S-1AH-GOR", vectors[0].header.at(0));

    ASSERT_LE(3, vectors[0].values.size());
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
