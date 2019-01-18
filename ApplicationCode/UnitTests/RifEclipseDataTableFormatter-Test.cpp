#include "gtest/gtest.h"

#include "RifEclipseDataTableFormatter.h"

#include <QString>
#include <QStringList>

TEST(RifEclipseDataTableFormatter, BasicUsage)
{
    QString tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);


    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Integer Number"),
        RifEclipseOutputTableColumn("IntNumer 2"),
        RifEclipseOutputTableColumn("IntNumer 3"),
    };

    formatter.header(header);

    formatter.add("well a");
    formatter.add(1);
    formatter.add(2);
    formatter.add(3);
    formatter.rowCompleted();

    formatter.add("well B");
    formatter.add(12);
    formatter.add(23);
    formatter.add(233);
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();

}


TEST(RifEclipseDataTableFormatter, NoPrefix)
{
    QString tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);

    formatter.setTableRowPrependText("   ");
    formatter.setTableRowLineAppendText("");


    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Integer Number", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("IntNumer 2", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("IntNumer 3", RifEclipseOutputTableDoubleFormatting(), RIGHT),
    };

    formatter.header(header);

    formatter.add("well a");
    formatter.add(1);
    formatter.add(2);
    formatter.add(3);
    formatter.rowCompleted();

    formatter.add("well B");
    formatter.add(12);
    formatter.add(231);
    formatter.add(23123);
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();

}

TEST(RifEclipseDataTableFormatter, LongLine)
{
    QString                                  tableText;
    QTextStream                              stream(&tableText);
    RifEclipseDataTableFormatter             formatter(stream);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("50 Character Well Name"),
        RifEclipseOutputTableColumn("10 Int #1", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #2", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #3", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #4", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #5", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #6", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #7", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #8", RifEclipseOutputTableDoubleFormatting(), RIGHT),
    };

    formatter.header(header);
    QString fiftyCharacterWellName = "01234567890123456789012345678901234567890123456789";
    formatter.add(fiftyCharacterWellName);
    for (int i = 0; i < 8; ++i)
    {
        formatter.add(std::numeric_limits<int>::max()); // 10 characters
    }
    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() +
        50 + 8 * 10 + formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_EQ(tableWidth, fullLineLength);
    EXPECT_GT(tableWidth, formatter.maxDataRowWidth());
    
    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for (QString line : tableLines)
    {
        std::cout << QString("Line: \"%1\"").arg(line).toStdString() << std::endl;
        if (!line.startsWith(formatter.commentPrefix()))
        {
            EXPECT_LE(line.length(), formatter.maxDataRowWidth());
        }
    }
}

TEST(RifEclipseDataTableFormatter, LongLine132)
{
    QString                      tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("10 Char"),
        RifEclipseOutputTableColumn("10 Int #1", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #2", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #3", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #4", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #5", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #6", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #7", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("I", RifEclipseOutputTableDoubleFormatting(), RIGHT),
    };

    formatter.header(header);
    QString tenCharacterWellName = "0123456789";
    formatter.add(tenCharacterWellName);
    for (int i = 0; i < 7; ++i)
    {
        formatter.add(std::numeric_limits<int>::max()); // 10 characters
    }
    formatter.add(11);

    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() + 10 + 7 * 10 + 2 +
                         formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_GE(tableWidth, fullLineLength);
    EXPECT_EQ(formatter.maxDataRowWidth(), fullLineLength);

    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for (QString line : tableLines)
    {
        std::cout << QString("Line: \"%1\"").arg(line).toStdString() << std::endl;
        if (line.startsWith("0"))
        {
            EXPECT_EQ(line.length(), formatter.maxDataRowWidth());
        }
    }
}

TEST(RifEclipseDataTableFormatter, LongLine133)
{
    QString                      tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("10 Char"),
        RifEclipseOutputTableColumn("10 Int #1", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #2", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #3", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #4", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #5", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #6", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("10 Int #7", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("I", RifEclipseOutputTableDoubleFormatting(), RIGHT),
    };

    formatter.header(header);
    QString fiftyCharacterWellName = "0123456789";
    formatter.add(fiftyCharacterWellName);
    for (int i = 0; i < 7; ++i)
    {
        formatter.add(std::numeric_limits<int>::max()); // 10 characters
    }
    formatter.add(111);

    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() + 10 + 7 * 10 + 3 +
                         formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_GE(tableWidth, fullLineLength);
    EXPECT_LT(formatter.maxDataRowWidth(), fullLineLength);

    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for (QString line : tableLines)
    {
        std::cout << QString("Line: \"%1\"").arg(line).toStdString() << std::endl;
        if (line.startsWith("0"))
        {
            EXPECT_LE(line.length(), formatter.maxDataRowWidth());
        }
    }
}