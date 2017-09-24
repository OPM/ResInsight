#include "gtest/gtest.h"

#include "RifColumnBasedAsciiParser.h"
#include "RifColumnBasedRsmspecParser.h"

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
TEST(RifColumnBasedRsmspecParserTest, smallTest)
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

    EXPECT_EQ("WLVP", tables.at(0).at(1).quantityName);
    EXPECT_EQ("P-9P", tables.at(1).at(1).wellName);
    EXPECT_NE("P-9P", tables.at(1).at(0).wellName);
}