#include "gtest/gtest.h"

#include "RifColumnBasedAsciiParser.h"

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
