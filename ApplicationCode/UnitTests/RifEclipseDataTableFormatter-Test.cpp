#include "gtest/gtest.h"

#include "RifEclipseDataTableFormatter.h"

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
    EXPECT_GT(tableWidth, RifEclipseDataTableFormatter::maxEclipseRowWidth());
    
    formatter.rowCompleted();
    formatter.tableCompleted();
    std::cout << tableText.toStdString() << std::endl;

    QStringList tableLines = tableText.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for (QString line : tableLines)
    {
        if (!line.startsWith(formatter.commentPrefix()))
        {
            EXPECT_LE(line.length(), RifEclipseDataTableFormatter::maxEclipseRowWidth());
        }
    }
}